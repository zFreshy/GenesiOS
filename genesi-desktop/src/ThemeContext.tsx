import React, { createContext, useContext, useState } from 'react';

type Theme = 'dark' | 'light';

interface ThemeContextType {
  theme: Theme;
  setTheme: (theme: Theme) => void;
  wallpaper: string;
  setWallpaper: (wallpaper: string) => void;
}

export const ThemeContext = createContext<ThemeContextType>({
  theme: 'dark',
  setTheme: () => {},
  wallpaper: '/wallpaper1.png',
  setWallpaper: () => {},
});

export const useTheme = () => useContext(ThemeContext);

export const ThemeProvider = ({ children }: { children: React.ReactNode }) => {
  const [theme, setTheme] = useState<Theme>('dark');
  const [wallpaper, setWallpaper] = useState<string>('/wallpaper1.png');

  return (
    <ThemeContext.Provider value={{ theme, setTheme, wallpaper, setWallpaper }}>
      {children}
    </ThemeContext.Provider>
  );
};