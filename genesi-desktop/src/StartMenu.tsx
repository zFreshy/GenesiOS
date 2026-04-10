import { motion, AnimatePresence } from 'framer-motion';
import { 
  Search, Power, User, Settings, Globe, Terminal,
  Image as ImageIcon, Folder, FileText, LayoutGrid, CheckCircle, ChevronRight
} from 'lucide-react';

interface StartMenuProps {
  show: boolean;
  onClose: () => void;
  onOpenApp: (id: string) => void;
  apps: any[];
  x: number;
}

const StartMenu = ({ show, onClose, onOpenApp, apps, x }: StartMenuProps) => {
  return (
    <AnimatePresence>
      {show && (
        <motion.div
          initial={{ opacity: 0, y: 50, x: "-50%", scale: 0.95 }}
          animate={{ opacity: 1, y: 0, x: "-50%", scale: 1 }}
          exit={{ opacity: 0, y: 50, x: "-50%", scale: 0.95 }}
          transition={{ type: "spring", stiffness: 300, damping: 30 }}
          onClick={(e) => e.stopPropagation()}
          className="absolute bottom-24 z-[9995] w-[600px] h-[700px] bg-[#1c1c1c]/90 backdrop-blur-3xl rounded-2xl border border-white/10 shadow-2xl flex flex-col text-white overflow-hidden"
          style={{ left: x }}
        >
          {/* Top Search Bar */}
          <div className="p-6 pb-2">
            <div className="relative flex items-center bg-white/10 rounded-full px-4 py-2 border border-white/5 focus-within:bg-white/15 focus-within:border-white/20 transition-colors">
              <Search size={18} className="text-white/60 mr-3" />
              <input 
                type="text" 
                placeholder="Search for apps, settings, and documents" 
                className="bg-transparent border-none outline-none w-full text-sm placeholder:text-white/50"
              />
            </div>
          </div>

          <div className="flex-1 overflow-y-auto custom-scrollbar px-8 pb-4">
            {/* Pinned Section */}
            <div className="mt-4">
              <div className="flex justify-between items-center mb-4">
                <h3 className="text-[13px] font-semibold text-white/90">Pinned</h3>
                <button className="text-[12px] bg-white/5 hover:bg-white/10 px-3 py-1 rounded-md transition-colors flex items-center gap-1">
                  All apps <ChevronRight size={14} />
                </button>
              </div>
              
              <div className="grid grid-cols-6 gap-y-6 gap-x-2">
                {/* Apps do sistema */}
                {apps.map(app => (
                  <div key={app.id} onClick={() => { onOpenApp(app.id); onClose(); }} className="flex flex-col items-center gap-2 cursor-pointer group rounded-lg p-2 hover:bg-white/5 transition-colors">
                    <div className={`w-10 h-10 rounded-xl ${app.color} flex justify-center items-center shadow-lg group-hover:scale-105 transition-transform`}>
                      <app.icon size={22} color="white" />
                    </div>
                    <span className="text-[11px] text-center text-white/80 group-hover:text-white leading-tight truncate w-full">{app.title.split(' ')[0]}</span>
                  </div>
                ))}
                
                {/* Apps mockados */}
                <div className="flex flex-col items-center gap-2 cursor-pointer group rounded-lg p-2 hover:bg-white/5 transition-colors">
                  <div className="w-10 h-10 rounded-xl bg-blue-600 flex justify-center items-center shadow-lg group-hover:scale-105 transition-transform"><Settings size={22} color="white" /></div>
                  <span className="text-[11px] text-center text-white/80 group-hover:text-white leading-tight truncate w-full">Settings</span>
                </div>
                <div className="flex flex-col items-center gap-2 cursor-pointer group rounded-lg p-2 hover:bg-white/5 transition-colors">
                  <div className="w-10 h-10 rounded-xl bg-yellow-500 flex justify-center items-center shadow-lg group-hover:scale-105 transition-transform"><Folder size={22} color="white" /></div>
                  <span className="text-[11px] text-center text-white/80 group-hover:text-white leading-tight truncate w-full">Files</span>
                </div>
                <div className="flex flex-col items-center gap-2 cursor-pointer group rounded-lg p-2 hover:bg-white/5 transition-colors">
                  <div className="w-10 h-10 rounded-xl bg-pink-500 flex justify-center items-center shadow-lg group-hover:scale-105 transition-transform"><ImageIcon size={22} color="white" /></div>
                  <span className="text-[11px] text-center text-white/80 group-hover:text-white leading-tight truncate w-full">Photos</span>
                </div>
              </div>
            </div>

            {/* Recommended Section */}
            <div className="mt-8">
              <div className="flex justify-between items-center mb-4">
                <h3 className="text-[13px] font-semibold text-white/90">Recommended</h3>
                <button className="text-[12px] text-white/60 hover:text-white transition-colors flex items-center gap-1">
                  Show all <ChevronRight size={14} />
                </button>
              </div>

              <div className="grid grid-cols-2 gap-3">
                <div className="flex items-center gap-3 p-3 rounded-lg hover:bg-white/5 cursor-pointer transition-colors">
                  <div className="w-8 h-8 rounded-lg bg-yellow-500/20 flex justify-center items-center text-yellow-500"><Folder size={16} /></div>
                  <div className="flex flex-col overflow-hidden">
                    <span className="text-[13px] truncate text-white/90">GenesiOS Source Code</span>
                    <span className="text-[11px] text-white/50">17m ago</span>
                  </div>
                </div>
                <div className="flex items-center gap-3 p-3 rounded-lg hover:bg-white/5 cursor-pointer transition-colors">
                  <div className="w-8 h-8 rounded-lg bg-blue-500/20 flex justify-center items-center text-blue-500"><FileText size={16} /></div>
                  <div className="flex flex-col overflow-hidden">
                    <span className="text-[13px] truncate text-white/90">testes.txt</span>
                    <span className="text-[11px] text-white/50">1h ago</span>
                  </div>
                </div>
                <div className="flex items-center gap-3 p-3 rounded-lg hover:bg-white/5 cursor-pointer transition-colors">
                  <div className="w-8 h-8 rounded-lg bg-purple-500/20 flex justify-center items-center text-purple-500"><CheckCircle size={16} /></div>
                  <div className="flex flex-col overflow-hidden">
                    <span className="text-[13px] truncate text-white/90">App Cert Kit</span>
                    <span className="text-[11px] text-white/50">Recently added</span>
                  </div>
                </div>
                <div className="flex items-center gap-3 p-3 rounded-lg hover:bg-white/5 cursor-pointer transition-colors">
                  <div className="w-8 h-8 rounded-lg bg-pink-500/20 flex justify-center items-center text-pink-500"><ImageIcon size={16} /></div>
                  <div className="flex flex-col overflow-hidden">
                    <span className="text-[13px] truncate text-white/90">Screenshot.png</span>
                    <span className="text-[11px] text-white/50">Yesterday at 4:20 PM</span>
                  </div>
                </div>
              </div>
            </div>

            {/* All Section (Categorized) - matching the user image exactly */}
            <div className="mt-8 mb-6">
              <div className="flex justify-between items-center mb-4">
                <h3 className="text-[13px] font-semibold text-white/90">All</h3>
                <button className="text-[12px] bg-white/5 hover:bg-white/10 px-3 py-1 rounded-md transition-colors flex items-center gap-1">
                  View: Category <ChevronRight size={14} className="rotate-90" />
                </button>
              </div>

              <div className="grid grid-cols-4 gap-4">
                {/* Category 1 */}
                <div className="flex flex-col items-center">
                  <div className="bg-white/5 rounded-2xl w-full aspect-square p-4 grid grid-cols-2 grid-rows-2 gap-2 hover:bg-white/10 transition-colors cursor-pointer">
                    <div className="w-full h-full bg-green-500 rounded-md flex items-center justify-center shadow-sm"><Globe size={14} color="white"/></div>
                    <div className="w-full h-full bg-green-600 rounded-md flex items-center justify-center shadow-sm"><span className="text-[10px] font-bold">W</span></div>
                    <div className="w-full h-full bg-blue-400 rounded-md flex items-center justify-center shadow-sm"><span className="text-[10px] font-bold">L</span></div>
                    <div className="w-full h-full bg-gray-700 rounded-md flex items-center justify-center shadow-sm"><span className="text-[10px] font-bold">S</span></div>
                  </div>
                  <span className="text-[11px] mt-2 text-white/70 font-medium">Other</span>
                </div>
                {/* Category 2 */}
                <div className="flex flex-col items-center">
                  <div className="bg-white/5 rounded-2xl w-full aspect-square p-4 grid grid-cols-2 grid-rows-2 gap-2 hover:bg-white/10 transition-colors cursor-pointer">
                    <div className="w-full h-full bg-yellow-500 rounded-md flex items-center justify-center shadow-sm"><Folder size={14} color="white"/></div>
                    <div className="w-full h-full bg-indigo-500 rounded-md flex items-center justify-center shadow-sm"><span className="text-[10px] font-bold">D</span></div>
                    <div className="w-full h-full bg-blue-500 rounded-md flex items-center justify-center shadow-sm"><span className="text-[10px] font-bold">G</span></div>
                    <div className="w-full h-full bg-teal-500 rounded-md flex items-center justify-center shadow-sm"><LayoutGrid size={14} color="white"/></div>
                  </div>
                  <span className="text-[11px] mt-2 text-white/70 font-medium">Productivity</span>
                </div>
                {/* Category 3 */}
                <div className="flex flex-col items-center">
                  <div className="bg-white/5 rounded-2xl w-full aspect-square p-4 grid grid-cols-2 grid-rows-2 gap-2 hover:bg-white/10 transition-colors cursor-pointer">
                    <div className="w-full h-full bg-red-600 rounded-md flex items-center justify-center shadow-sm"><span className="text-[10px] font-bold text-white">IJ</span></div>
                    <div className="w-full h-full bg-blue-600 rounded-md flex items-center justify-center shadow-sm"><span className="text-[10px] font-bold text-white">VS</span></div>
                    <div className="w-full h-full bg-gray-800 rounded-md flex items-center justify-center shadow-sm"><Terminal size={14} color="white"/></div>
                    <div className="w-full h-full bg-orange-600 rounded-md flex items-center justify-center shadow-sm"><span className="text-[10px] font-bold text-white">Fz</span></div>
                  </div>
                  <span className="text-[11px] mt-2 text-white/70 font-medium">Developer Tools</span>
                </div>
                {/* Category 4 */}
                <div className="flex flex-col items-center">
                  <div className="bg-white/5 rounded-2xl w-full aspect-square p-4 grid grid-cols-2 grid-rows-2 gap-2 hover:bg-white/10 transition-colors cursor-pointer">
                    <div className="w-full h-full bg-blue-500 rounded-md flex items-center justify-center shadow-sm"><Settings size={14} color="white"/></div>
                    <div className="w-full h-full bg-purple-600 rounded-md flex items-center justify-center shadow-sm"><span className="text-[10px] font-bold text-white">EA</span></div>
                    <div className="w-full h-full bg-indigo-600 rounded-md flex items-center justify-center shadow-sm"><span className="text-[10px] font-bold text-white">B</span></div>
                    <div className="w-full h-full bg-green-500 rounded-md flex items-center justify-center shadow-sm"><span className="text-[10px] font-bold text-white">X</span></div>
                  </div>
                  <span className="text-[11px] mt-2 text-white/70 font-medium">Games</span>
                </div>
              </div>
            </div>

          </div>

          {/* Bottom Bar */}
          <div className="h-16 bg-black/40 border-t border-white/5 flex items-center justify-between px-6 mt-auto">
            <div className="flex items-center gap-3 hover:bg-white/5 p-2 rounded-lg cursor-pointer transition-colors">
              <div className="w-8 h-8 rounded-full bg-gradient-to-tr from-pink-500 to-violet-500 flex items-center justify-center overflow-hidden border border-white/20">
                <User size={18} className="text-white/80" />
              </div>
              <span className="text-[13px] font-medium">Matheus Vinícius</span>
            </div>
            
            <button className="w-10 h-10 flex items-center justify-center hover:bg-white/10 rounded-full transition-colors group">
              <Power size={18} className="text-white/80 group-hover:text-red-400 transition-colors" />
            </button>
          </div>
        </motion.div>
      )}
    </AnimatePresence>
  );
};

export default StartMenu;