import { useState, useEffect } from 'react';
import { invoke } from '@tauri-apps/api/core';
import { useTheme } from './ThemeContext';

export default function VideoPlayer({ filePath, fileName }: { filePath: string, fileName: string }) {
  const { theme } = useTheme();
  const [videoUrl, setVideoUrl] = useState<string | null>(null);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    const loadVideo = async () => {
      try {
        const bytes: number[] = await invoke('read_file_bytes', { path: filePath });
        const blob = new Blob([new Uint8Array(bytes)]);
        setVideoUrl(URL.createObjectURL(blob));
      } catch (err) {
        setError('Failed to load video');
        console.error(err);
      }
    };
    loadVideo();
  }, [filePath]);

  if (error) return <div className={`flex h-full items-center justify-center ${theme === 'light' ? 'text-black' : 'text-white'}`}>{error}</div>;
  if (!videoUrl) return <div className={`flex h-full items-center justify-center ${theme === 'light' ? 'text-black' : 'text-white'}`}>Loading...</div>;

  return (
    <div className={`w-full h-full flex flex-col ${theme === 'light' ? 'bg-[#f5f5f5]' : 'bg-[#1e1e1e]'}`}>
      <div className={`h-10 flex items-center px-4 shrink-0 border-b ${theme === 'light' ? 'border-black/10' : 'border-white/10'}`}>
        <span className={`text-sm ${theme === 'light' ? 'text-black/70' : 'text-white/70'} truncate`}>{fileName}</span>
      </div>
      <div className="flex-1 flex items-center justify-center overflow-hidden bg-black relative">
        <video 
          src={videoUrl} 
          controls 
          autoPlay 
          className="max-w-full max-h-full" 
        />
      </div>
    </div>
  );
}