import { useState, useEffect } from 'react';
import { invoke } from '@tauri-apps/api/core';
import { useTheme } from './ThemeContext';
import { Save } from 'lucide-react';

export default function TextEditor({ filePath, fileName }: { filePath: string, fileName: string }) {
  const { theme } = useTheme();
  const [content, setContent] = useState<string>('');
  const [error, setError] = useState<string | null>(null);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    const loadText = async () => {
      try {
        const bytes: number[] = await invoke('read_file_bytes', { path: filePath });
        const text = new TextDecoder().decode(new Uint8Array(bytes));
        setContent(text);
      } catch (err) {
        setError('Failed to load text file');
        console.error(err);
      } finally {
        setLoading(false);
      }
    };
    loadText();
  }, [filePath]);

  if (loading) return <div className={`flex h-full items-center justify-center ${theme === 'light' ? 'text-black' : 'text-white'}`}>Loading...</div>;
  if (error) return <div className={`flex h-full items-center justify-center ${theme === 'light' ? 'text-black' : 'text-white'}`}>{error}</div>;

  return (
    <div className={`w-full h-full flex flex-col ${theme === 'light' ? 'bg-white' : 'bg-[#1e1e1e]'}`}>
      {/* Toolbar */}
      <div className={`h-12 flex items-center px-4 shrink-0 border-b gap-4 ${theme === 'light' ? 'bg-[#f5f5f5] border-black/10' : 'bg-[#2d2d2d] border-white/10'}`}>
        <button 
          className={`flex items-center gap-2 px-3 py-1.5 rounded-md transition-colors text-[13px] font-medium ${
            theme === 'light' ? 'hover:bg-black/10 text-black/80' : 'hover:bg-white/10 text-white/80'
          }`}
          onClick={() => alert("Save functionality would go here.")}
        >
          <Save size={16} /> Save
        </button>
        <span className={`text-sm ml-auto ${theme === 'light' ? 'text-black/50' : 'text-white/50'} truncate`}>
          {fileName}
        </span>
      </div>
      
      {/* Editor Area */}
      <div className="flex-1 overflow-hidden p-2 relative">
        <textarea 
          value={content}
          onChange={(e) => setContent(e.target.value)}
          spellCheck={false}
          className={`w-full h-full resize-none outline-none font-mono text-sm p-4 rounded-md custom-scrollbar ${
            theme === 'light' 
              ? 'bg-white text-black border border-black/5' 
              : 'bg-[#191919] text-white border border-white/5'
          }`}
        />
      </div>
    </div>
  );
}