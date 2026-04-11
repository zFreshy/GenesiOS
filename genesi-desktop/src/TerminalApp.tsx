import React, { useState, useRef, useEffect } from 'react';
import { Command } from '@tauri-apps/plugin-shell';

interface TerminalLine {
  text: string;
  isInput: boolean;
  dir?: string;
}

export default function TerminalApp() {
  const [lines, setLines] = useState<TerminalLine[]>([
    { text: 'Genesi OS Terminal - v1.0.0', isInput: false },
    { text: 'Type "help" for a list of available commands.', isInput: false }
  ]);
  const [input, setInput] = useState('');
  const [cwd, setCwd] = useState('C:\\');
  const [history, setHistory] = useState<string[]>([]);
  const [historyIndex, setHistoryIndex] = useState(-1);
  const bottomRef = useRef<HTMLDivElement>(null);
  const inputRef = useRef<HTMLInputElement>(null);

  useEffect(() => {
    bottomRef.current?.scrollIntoView({ behavior: 'smooth' });
  }, [lines]);

  // Função para executar comandos no PowerShell
  const executeCommand = async (cmdStr: string) => {
    if (!cmdStr.trim()) return;

    setLines(prev => [...prev, { text: cmdStr, isInput: true, dir: cwd }]);
    setHistory(prev => [cmdStr, ...prev]); // mais recentes no topo
    setHistoryIndex(-1);

    const args = cmdStr.trim().split(' ');
    const commandName = args[0].toLowerCase();

    if (commandName === 'clear' || commandName === 'cls') {
      setLines([]);
      return;
    }

    if (commandName === 'help') {
      setLines(prev => [...prev, 
        { text: 'Available commands:', isInput: false },
        { text: '  clear/cls - Clear terminal', isInput: false },
        { text: '  (other commands will be passed to Windows PowerShell)', isInput: false }
      ]);
      return;
    }

    try {
      // Para manter o estado do CWD entre os comandos, passamos o cd primeiro
      const shellCmd = Command.create('powershell', [
        '-NoProfile', 
        '-Command', 
        `Set-Location -Path "${cwd}"; ${cmdStr}; ""`
      ]);
      
      const output = await shellCmd.execute();

      if (output.stdout && output.stdout.trim() !== '') {
        const outLines = output.stdout.split('\n');
        setLines(prev => [...prev, ...outLines.map(text => ({ text, isInput: false }))]);
      }
      if (output.stderr && output.stderr.trim() !== '') {
        const errLines = output.stderr.split('\n');
        setLines(prev => [...prev, ...errLines.map(text => ({ text: `[Error] ${text}`, isInput: false }))]);
      }
      
      // Sempre tentamos atualizar o CWD no caso do usuário ter digitado 'cd'
      if (commandName === 'cd') {
        const pwdCmd = Command.create('powershell', [
          '-NoProfile', 
          '-Command', 
          `Set-Location -Path "${cwd}"; ${cmdStr}; (Get-Location).Path`
        ]);
        const pwdOut = await pwdCmd.execute();
        if (pwdOut.stdout && pwdOut.stdout.trim() !== '') {
           setCwd(pwdOut.stdout.trim().split('\n').pop() || cwd);
        }
      }

    } catch (e: any) {
      setLines(prev => [...prev, { text: `Command failed: ${e.message || String(e)}`, isInput: false }]);
    }
  };

  const handleKeyDown = (e: React.KeyboardEvent<HTMLInputElement>) => {
    if (e.key === 'Enter') {
      executeCommand(input);
      setInput('');
    } else if (e.key === 'ArrowUp') {
      e.preventDefault();
      if (history.length > 0) {
        const newIdx = historyIndex + 1 < history.length ? historyIndex + 1 : historyIndex;
        setHistoryIndex(newIdx);
        setInput(history[newIdx]);
      }
    } else if (e.key === 'ArrowDown') {
      e.preventDefault();
      if (historyIndex > 0) {
        const newIdx = historyIndex - 1;
        setHistoryIndex(newIdx);
        setInput(history[newIdx]);
      } else if (historyIndex === 0) {
        setHistoryIndex(-1);
        setInput('');
      }
    }
  };

  return (
    <div 
      className="flex flex-col w-full h-full bg-[#1e1e1e] text-[#cccccc] font-mono text-sm p-4 overflow-y-auto cursor-text" 
      onClick={() => inputRef.current?.focus()}
    >
      {lines.map((line, i) => (
        <div key={i} className="whitespace-pre-wrap break-all leading-snug mb-1">
          {line.isInput ? (
            <div className="flex gap-2 text-green-400 mt-2">
              <span className="text-blue-400 shrink-0">{line.dir}&gt;</span>
              <span className="text-white">{line.text}</span>
            </div>
          ) : (
            <span className={line.text.startsWith('[Error]') ? 'text-red-400' : 'text-gray-300'}>
              {line.text}
            </span>
          )}
        </div>
      ))}
      <div className="flex gap-2 text-green-400 mt-2 items-center">
        <span className="text-blue-400 shrink-0">{cwd}&gt;</span>
        <input
          ref={inputRef}
          type="text"
          value={input}
          onChange={(e) => setInput(e.target.value)}
          onKeyDown={handleKeyDown}
          className="flex-1 bg-transparent outline-none border-none text-white font-mono"
          autoFocus
          spellCheck="false"
          autoComplete="off"
        />
      </div>
      <div ref={bottomRef} className="h-4" />
    </div>
  );
}