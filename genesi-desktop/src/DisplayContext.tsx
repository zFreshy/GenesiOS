import React, { createContext, useContext, useState, useEffect } from 'react';
import { getCurrentWindow } from '@tauri-apps/api/window';
import { availableMonitors, currentMonitor, Monitor } from '@tauri-apps/api/window';
import { LazyStore } from '@tauri-apps/plugin-store';
import { isTauri } from '@tauri-apps/api/core';

export interface Display {
  id: string;
  name: string;
  physicalX: number;
  physicalY: number;
  physicalWidth: number;
  physicalHeight: number;
  logicalX: number;
  logicalY: number;
  logicalWidth: number;
  logicalHeight: number;
  isPrimary: boolean;
  scaleFactor: number;
}

interface DisplayContextType {
  displays: Display[];
  setDisplays: (displays: Display[]) => void;
  updateDisplayLayout: (id: string, layout: Partial<Display>) => void;
  isMultiMonitor: boolean;
}

export const DisplayContext = createContext<DisplayContextType>({
  displays: [],
  setDisplays: () => {},
  updateDisplayLayout: () => {},
  isMultiMonitor: false,
});

export const useDisplay = () => useContext(DisplayContext);

const store = new LazyStore('monitors.json');

export const DisplayProvider = ({ children }: { children: React.ReactNode }) => {
  const [displays, setDisplays] = useState<Display[]>([]);
  const isMultiMonitor = displays.length > 1;

  useEffect(() => {
    const initDisplays = async () => {
      if (!isTauri()) {
        console.warn('Not running in Tauri. Using mock displays.');
        setDisplays([{
          id: 'display-0', name: 'Mock Monitor',
          physicalX: 0, physicalY: 0, physicalWidth: window.innerWidth, physicalHeight: window.innerHeight,
          logicalX: 0, logicalY: 0, logicalWidth: window.innerWidth, logicalHeight: window.innerHeight,
          isPrimary: true, scaleFactor: 1
        }]);
        return;
      }

      const windowObj = getCurrentWindow();
      const monitors = await availableMonitors();
      
      if (monitors.length === 0) return;

      // Find bounding box of all physical monitors
      let minX = monitors[0].position.x;
      let minY = monitors[0].position.y;
      let maxX = monitors[0].position.x + monitors[0].size.width;
      let maxY = monitors[0].position.y + monitors[0].size.height;

      for (const m of monitors) {
        minX = Math.min(minX, m.position.x);
        minY = Math.min(minY, m.position.y);
        maxX = Math.max(maxX, m.position.x + m.size.width);
        maxY = Math.max(maxY, m.position.y + m.size.height);
      }

      const totalWidth = maxX - minX;
      const totalHeight = maxY - minY;
      
      // Compensação de borda invisível do Windows 11 para janelas transparentes
      // Margem de segurança generosa para garantir que a janela cubra tudo (o SO recorta o que sobrar fora do monitor)
      const padding = 20;

      try {
        await windowObj.setPosition({ type: 'Physical', x: minX - padding, y: minY - padding } as any);
        await windowObj.setSize({ type: 'Physical', width: totalWidth + (padding * 2), height: totalHeight + (padding * 2) } as any);
      } catch (e) {
        console.error('Failed to resize window to span monitors', e);
      }

      // Aguarda o SO atualizar o fator de escala da janela após redimensionar por múltiplos monitores
      await new Promise(resolve => setTimeout(resolve, 100));
      
      const windowScaleFactor = await windowObj.scaleFactor();
      const innerPos = await windowObj.innerPosition();

      const initialDisplays: Display[] = await Promise.all(monitors.map(async (m, i) => {
        const sf = windowScaleFactor; // USAR O SCALE FACTOR DA JANELA GLOBAL PARA MANTER O GRID CONTÍNUO
        
        // Posições físicas relativas à área de cliente real da janela mãe
        const localPx = m.position.x - innerPos.x;
        const localPy = m.position.y - innerPos.y;

        // Converter pixels físicos (monitor) para pixels lógicos (CSS React)
        const lw = m.size.width / sf;
        const lh = m.size.height / sf;
        const lx = localPx / sf;
        const ly = localPy / sf;

        const savedIsPrimary = await store.get<boolean>(`display_${i}_isPrimary`);

        return {
          id: `display-${i}`,
          name: m.name || `Monitor ${i + 1}`,
          physicalX: m.position.x,
          physicalY: m.position.y,
          physicalWidth: m.size.width,
          physicalHeight: m.size.height,
          logicalX: lx, 
          logicalY: ly,
          logicalWidth: lw,
          logicalHeight: lh,
          isPrimary: savedIsPrimary !== null && savedIsPrimary !== undefined ? savedIsPrimary : (i === 0),
          scaleFactor: sf,
        };
      }));

      setDisplays(initialDisplays);
    };

    initDisplays();
  }, []);

  const updateDisplayLayout = async (id: string, layout: Partial<Display>) => {
    const newDisplays = displays.map(d => {
      if (d.id === id) {
        return { ...d, ...layout };
      }
      if (layout.isPrimary && d.id !== id) {
        return { ...d, isPrimary: false };
      }
      return d;
    });

    setDisplays(newDisplays);

    if (!isTauri()) return;

    // Save to store
    for (let i = 0; i < newDisplays.length; i++) {
      const d = newDisplays[i];
      await store.set(`display_${i}_isPrimary`, d.isPrimary);
    }
    await store.save();
  };

  return (
    <DisplayContext.Provider value={{ displays, setDisplays, updateDisplayLayout, isMultiMonitor }}>
      {children}
    </DisplayContext.Provider>
  );
};
