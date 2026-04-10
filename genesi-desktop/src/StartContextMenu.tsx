import { motion } from 'framer-motion';
import { ChevronRight } from 'lucide-react';

interface StartContextMenuProps {
  onClose: () => void;
  onOpenApp: (id: string) => void;
  x: number;
  y: number;
}

const StartContextMenu = ({ onClose, onOpenApp, x, y }: StartContextMenuProps) => {
  return (
    <motion.div
      initial={{ opacity: 0, y: 20, scale: 0.95 }}
      animate={{ opacity: 1, y: 0, scale: 1 }}
      exit={{ opacity: 0, y: 20, scale: 0.95 }}
      transition={{ type: "spring", stiffness: 300, damping: 30 }}
      onClick={(e) => e.stopPropagation()}
      className="absolute z-[9995] w-[280px] bg-[#1c1c1c]/95 backdrop-blur-3xl rounded-xl border border-white/10 shadow-2xl flex flex-col text-white py-2 text-[13px] font-medium"
      style={{ left: x, top: y - 480 /* Altura aproximada do menu para ele nascer pra cima */ }}
    >
      <div className="flex flex-col">
            <MenuItem label="Installed apps" />
            <MenuItem label="Power Options" />
            <MenuItem label="Event Viewer" />
            <MenuItem label="System" />
            <MenuItem label="Device Manager" />
            <MenuItem label="Network Connections" />
            <MenuItem label="Disk Management" />
            <MenuItem label="Computer Management" />
            <MenuItem label="Terminal" />
            <MenuItem label="Terminal (Admin)" />
            <MenuSeparator />
            <MenuItem label="Task Manager" />
            <MenuItem label="Settings" onClick={() => { onOpenApp('settings'); onClose(); }} />
            <MenuItem label="File Explorer" onClick={() => { onOpenApp('files'); onClose(); }} />
            <MenuItem label="Search" />
            <MenuItem label="Run" />
            <MenuSeparator />
            <MenuItem label="Shut down or sign out" hasSubmenu />
            <MenuItem label="Desktop" />
          </div>
    </motion.div>
  );
};

const MenuItem = ({ label, onClick, hasSubmenu = false }: { label: string, onClick?: () => void, hasSubmenu?: boolean }) => (
  <div 
    onClick={onClick}
    className="flex items-center justify-between px-4 py-1.5 hover:bg-white/10 cursor-pointer transition-colors"
  >
    <span>{label}</span>
    {hasSubmenu && <ChevronRight size={14} className="text-white/60" />}
  </div>
);

const MenuSeparator = () => (
  <div className="h-[1px] bg-white/10 my-1 mx-2" />
);

export default StartContextMenu;