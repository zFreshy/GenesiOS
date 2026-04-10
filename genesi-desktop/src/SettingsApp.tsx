import { useState, useEffect } from 'react';
import { motion, AnimatePresence, useDragControls } from 'framer-motion';
import { 
  Search, Settings, Home, Users, Activity, Trophy, Calendar, Lightbulb, 
  FileSpreadsheet, Grid, Settings2, User, Sliders, AppWindow, ArrowUpCircle, 
  Shield, LayoutTemplate, CreditCard, Map, Check, ChevronDown, Plus, Zap, Layers, TreePine, Hexagon, ChevronRight,
  Image as ImageIcon, X, Monitor, Paintbrush
} from 'lucide-react';
import { invoke } from '@tauri-apps/api/core';

import { useTheme } from './ThemeContext';
import { useDisplay } from './DisplayContext';

import { FileExplorerBase } from './FileExplorer';

const SettingsApp = () => {
  const { theme, setTheme, wallpaperHistory, setWallpaper } = useTheme();
  const { displays, updateDisplayLayout, isMultiMonitor } = useDisplay();
  
  const [mobilePush, setMobilePush] = useState(true);
  const [desktopPush, setDesktopPush] = useState(true);
  const [emailPush, setEmailPush] = useState(false);
  const [twoFactor, setTwoFactor] = useState(true);
  const [showFilePicker, setShowFilePicker] = useState(false);
  const [activeTab, setActiveTab] = useState<'general' | 'display' | 'personalization'>('general');
  const [targetMonitorId, setTargetMonitorId] = useState<string>('all');
  const [historyBlobs, setHistoryBlobs] = useState<Record<string, string>>({});

  // Helper to load image bytes to blob URL for history
  useEffect(() => {
    const loadBlobs = async () => {
      const newBlobs: Record<string, string> = {};
      for (const path of wallpaperHistory) {
        if (path.startsWith('/')) {
          newBlobs[path] = path;
        } else {
          try {
            const bytes: number[] = await invoke('read_file_bytes', { path });
            const blob = new Blob([new Uint8Array(bytes)]);
            newBlobs[path] = URL.createObjectURL(blob);
          } catch (e) {
            console.error('Failed to load history blob:', e);
          }
        }
      }
      setHistoryBlobs(newBlobs);
    };
    loadBlobs();
  }, [wallpaperHistory]);

  return (
    <div className={`flex w-full h-full ${theme === 'light' ? 'bg-[#f5f5f5] text-black/90' : 'bg-[#121212] text-white/90'} overflow-hidden rounded-b-xl select-none font-sans border ${theme === 'light' ? 'border-black/5' : 'border-white/5'}`}>
      
      {/* 1. Leftmost Mini Sidebar (Brand/Tools) */}
      <div className={`w-[60px] h-full ${theme === 'light' ? 'bg-[#ebebeb] border-black/5' : 'bg-[#0a0a0a] border-white/5'} border-r flex flex-col items-center py-6 gap-6 shrink-0 z-20`}>
        <div className="w-10 h-10 rounded-xl bg-blue-600/20 text-blue-500 flex items-center justify-center cursor-pointer hover:bg-blue-600/30 transition-colors">
          <Hexagon size={22} fill="currentColor" />
        </div>
        <div className="flex flex-col gap-4 mt-4">
          <IconBtn icon={Zap} active theme={theme} />
          <IconBtn icon={Layers} theme={theme} />
          <IconBtn icon={TreePine} theme={theme} />
        </div>
        <div className="mt-auto">
          <div className={`w-10 h-10 rounded-full border ${theme === 'light' ? 'border-black/10 text-black/40 hover:bg-black/5' : 'border-white/10 text-white/40 hover:bg-white/5'} flex items-center justify-center cursor-pointer transition-colors`}>
            <Plus size={20} />
          </div>
        </div>
      </div>

      {/* 2. Secondary Sidebar (Main Navigation) */}
      <div className={`w-[240px] h-full ${theme === 'light' ? 'bg-[#f0f0f0] border-black/5' : 'bg-[#0f0f0f] border-white/5'} border-r flex flex-col shrink-0 z-10 hidden md:flex`}>
        {/* User Profile Area */}
        <div className={`h-[72px] flex items-center px-5 border-b ${theme === 'light' ? 'border-black/5 hover:bg-black/5' : 'border-white/5 hover:bg-white/5'} gap-3 cursor-pointer transition-colors`}>
          <img src="https://i.pravatar.cc/150?img=11" alt="User" className={`w-10 h-10 rounded-full border ${theme === 'light' ? 'border-black/10' : 'border-white/10'}`} />
          <div className="flex flex-col overflow-hidden">
            <span className="font-semibold text-[14px] truncate">Rafiqur...</span>
            <span className={`text-[11px] ${theme === 'light' ? 'text-black/50' : 'text-white/40'} truncate`}>rafiqur51@jira.com</span>
          </div>
          <Settings size={16} className={`${theme === 'light' ? 'text-black/40' : 'text-white/40'} ml-auto`} />
        </div>

        {/* Navigation Links */}
        <div className="flex-1 overflow-y-auto custom-scrollbar px-3 py-5 flex flex-col gap-6">
          
          <NavSection title="ANALYTICS" theme={theme}>
            <NavItem icon={Home} label="Overview" theme={theme} />
            <NavItem icon={Users} label="Team Insights" theme={theme} />
            <NavItem icon={Activity} label="Engagement" theme={theme} />
            <NavItem icon={Trophy} label="Leaderboard" theme={theme} />
          </NavSection>

          <NavSection title="CONTEXT" theme={theme}>
            <NavItem icon={Calendar} label="Calendar Events" theme={theme} />
            <NavItem icon={Lightbulb} label="Insights" theme={theme} />
            <NavItem icon={FileSpreadsheet} label="Spreadsheet" theme={theme} />
          </NavSection>

          <NavSection title="OTHERS" theme={theme}>
            <NavItem icon={Grid} label="Apps" theme={theme} />
            <NavItem icon={Settings2} label="Properties" theme={theme} />
            <NavItem icon={Settings} label="Settings" active theme={theme} />
          </NavSection>

        </div>
      </div>

      {/* 3. Main Content Area (Topbar + Content) */}
      <div className={`flex-1 flex flex-col h-full ${theme === 'light' ? 'bg-[#f5f5f5]' : 'bg-[#121212]'} overflow-hidden`}>
        
        {/* Topbar */}
        <div className={`h-[72px] border-b ${theme === 'light' ? 'border-black/5 bg-[#f0f0f0]/50' : 'border-white/5 bg-[#0f0f0f]/50'} flex items-center px-8 justify-between shrink-0 backdrop-blur-md`}>
          {/* Search */}
          <div className={`relative flex items-center ${theme === 'light' ? 'bg-[#ffffff] border-black/10' : 'bg-[#1a1a1a] border-white/5'} rounded-lg px-3 py-2 w-[300px] border focus-within:border-blue-500/50 transition-colors`}>
            <Search size={16} className={`${theme === 'light' ? 'text-black/40' : 'text-white/40'} mr-2`} />
            <input 
              type="text" 
              placeholder="Search item" 
              className={`bg-transparent border-none outline-none w-full text-[13px] ${theme === 'light' ? 'placeholder:text-black/40 text-black' : 'placeholder:text-white/40 text-white'}`}
            />
            <span className={`text-[10px] ${theme === 'light' ? 'text-black/40 border-black/10' : 'text-white/30 border-white/10'} border px-1.5 py-0.5 rounded ml-auto`}>⌘K</span>
          </div>

          {/* Right Area (Profile without bell) */}
          <div className="flex items-center gap-4">
            <img src="https://i.pravatar.cc/150?img=11" alt="Profile" className={`w-8 h-8 rounded-full border ${theme === 'light' ? 'border-black/10' : 'border-white/10'} cursor-pointer hover:opacity-80 transition-opacity`} />
          </div>
        </div>

        {/* Content Body (Third Sidebar + Actual Settings) */}
        <div className="flex-1 flex h-[calc(100%-72px)]">
          
          {/* Third Sidebar (Sub-navigation) */}
          <div className={`w-[240px] h-full border-r ${theme === 'light' ? 'border-black/5 bg-[#f5f5f5]' : 'border-white/5 bg-[#121212]'} flex flex-col shrink-0 hidden lg:flex`}>
            <div className="p-8 pb-4">
              <h2 className="text-[22px] font-semibold flex items-center gap-2">
                Settings <ChevronRight size={16} className={`${theme === 'light' ? 'text-black/40' : 'text-white/40'}`} /> <span className={`${theme === 'light' ? 'text-black/60' : 'text-white/60'} text-[15px] font-normal capitalize`}>{activeTab}</span>
              </h2>
            </div>
            <div className="flex-1 overflow-y-auto custom-scrollbar px-5 pb-6 flex flex-col gap-6">
              
              <NavSection title="ACCOUNT" theme={theme}>
                <NavItem icon={User} label="My Profile" compact theme={theme} />
                <NavItem icon={Home} label="General" active={activeTab === 'general'} onClick={() => setActiveTab('general')} compact theme={theme} />
                <NavItem icon={Monitor} label="Display" active={activeTab === 'display'} onClick={() => setActiveTab('display')} compact theme={theme} />
                <NavItem icon={Paintbrush} label="Personalization" active={activeTab === 'personalization'} onClick={() => setActiveTab('personalization')} compact theme={theme} />
                <NavItem icon={Sliders} label="Preferences" compact theme={theme} />
                <NavItem icon={AppWindow} label="Applications" compact theme={theme} />
              </NavSection>

              <NavSection title="WORKSPACE" theme={theme}>
                <NavItem icon={Settings} label="Settings" compact theme={theme} />
                <NavItem icon={Users} label="Members" compact theme={theme} />
                <NavItem icon={ArrowUpCircle} label="Upgrade" compact theme={theme} />
                <NavItem icon={Shield} label="Security" compact theme={theme} />
                <NavItem icon={LayoutTemplate} label="Templates" compact theme={theme} />
                <NavItem icon={CreditCard} label="Billing" compact theme={theme} />
                <NavItem icon={Map} label="Roadmaps" compact theme={theme} />
              </NavSection>

            </div>
          </div>

          {/* Actual Settings Form */}
          <div className="flex-1 h-full overflow-y-auto custom-scrollbar p-8 lg:p-12">
            <div className="max-w-3xl">
              
              {activeTab === 'general' && (
                <>
                  {/* Notifications Section */}
                  <h3 className="text-xl font-semibold mb-6 text-white/90">My Notifications</h3>
                  
                  <div className="flex justify-between items-center mb-4">
                    <span className="text-[14px] font-semibold">Notify me when...</span>
                    <span className="text-[13px] text-blue-500 cursor-pointer hover:underline">About notifications?</span>
                  </div>
                  
                  <div className="flex flex-col gap-3 mb-10">
                    <CustomCheckbox label="Daily productivity update" checked theme={theme} />
                    <CustomCheckbox label="New event created" checked theme={theme} />
                    <CustomCheckbox label="When added on new team" checked theme={theme} />
                  </div>

                  <div className="flex flex-col gap-8 mb-12">
                    <ToggleRow 
                      title="Mobile push notifications" 
                      desc="Receive push notification whenever your organisation requires your attentions" 
                      checked={mobilePush} onChange={() => setMobilePush(!mobilePush)} theme={theme}
                    />
                    <ToggleRow 
                      title="Desktop Notification" 
                      desc="Receive desktop notification whenever your organisation requires your attentions" 
                      checked={desktopPush} onChange={() => setDesktopPush(!desktopPush)} theme={theme}
                    />
                    <ToggleRow 
                      title="Email Notification" 
                      desc="Receive email whenever your organisation requires your attentions" 
                      checked={emailPush} onChange={() => setEmailPush(!emailPush)} theme={theme}
                    />
                  </div>

                  {/* Settings Section */}
                  <div className={`w-full h-[1px] ${theme === 'light' ? 'bg-black/5' : 'bg-white/5'} mb-8`}></div>
                  <h3 className={`text-xl font-semibold mb-6 ${theme === 'light' ? 'text-black/90' : 'text-white/90'}`}>My Settings</h3>

                  <div className="flex flex-col gap-8 pb-10">
                    <DropdownRow 
                      title="Appearance" 
                      desc="Customize how the theme looks on your device." 
                      value={theme === 'dark' ? 'Dark' : 'Light'}
                      onClick={() => setTheme(theme === 'dark' ? 'light' : 'dark')}
                      theme={theme}
                    />

                    <ToggleRow 
                      title="Two-factor authentication" 
                      desc="Keep your account secure by enabling 2FA via SMS or using a temporary one-time passcode (TOTP)." 
                      checked={twoFactor} onChange={() => setTwoFactor(!twoFactor)} theme={theme}
                    />
                    <DropdownRow 
                      title="Language" 
                      desc="Customize the language of the system." 
                      value="English" 
                      theme={theme}
                    />
                  </div>
                </>
              )}

              {activeTab === 'personalization' && (
                <>
                  <h3 className={`text-xl font-semibold mb-6 ${theme === 'light' ? 'text-black/90' : 'text-white/90'}`}>Personalization</h3>
                  
                  <div className="flex flex-col gap-4 mb-8">
                    <span className={`text-[14px] ${theme === 'light' ? 'text-black/80' : 'text-white/80'}`}>System Theme</span>
                    <DropdownRow 
                      title="Appearance" 
                      desc="Customize how the theme looks on your device." 
                      value={theme === 'dark' ? 'Dark' : 'Light'}
                      onClick={() => setTheme(theme === 'dark' ? 'light' : 'dark')}
                      theme={theme}
                    />
                  </div>

                  <div className={`w-full h-[1px] ${theme === 'light' ? 'bg-black/5' : 'bg-white/5'} mb-8`}></div>

                  <div className="flex flex-col gap-4 mb-6">
                    <span className={`text-[14px] ${theme === 'light' ? 'text-black/80' : 'text-white/80'}`}>Background</span>
                    <span className={`text-[13px] ${theme === 'light' ? 'text-black/50' : 'text-white/40'}`}>Personalize your desktop background.</span>
                  </div>

                  {/* Monitor Selection Dropdown */}
                  {isMultiMonitor && (
                    <div className={`flex items-center justify-between gap-4 mb-6 ${theme === 'light' ? 'bg-[#f0f0f0] border-black/5' : 'bg-[#1a1a1a]/50 border-white/5'} p-4 rounded-xl border`}>
                      <div className="flex flex-col">
                        <span className={`text-[14px] font-medium ${theme === 'light' ? 'text-black/90' : 'text-white/90'}`}>Choose a display</span>
                        <span className={`text-[13px] ${theme === 'light' ? 'text-black/50' : 'text-white/40'}`}>Select which monitor to apply the background.</span>
                      </div>
                      <select
                        value={targetMonitorId}
                        onChange={(e) => setTargetMonitorId(e.target.value)}
                        className={`${theme === 'light' ? 'bg-white border-black/10 text-black' : 'bg-[#2d2d2d] border-white/10 text-white'} border rounded-lg px-3 py-2 text-[13px] outline-none focus:border-blue-500/50 transition-colors cursor-pointer min-w-[150px]`}
                      >
                        <option value="all">All Displays</option>
                        {displays.map((d) => (
                          <option key={d.id} value={d.id}>{d.name} {d.isPrimary ? '(Primary)' : ''}</option>
                        ))}
                      </select>
                    </div>
                  )}

                  {/* Recent Wallpapers (History) */}
                  <div className="flex flex-col gap-4 mb-8">
                    <span className={`text-[13px] ${theme === 'light' ? 'text-black/60' : 'text-white/60'}`}>Recent images</span>
                    <div className="flex flex-wrap gap-4">
                      {wallpaperHistory.map((path, index) => (
                        <div 
                          key={index}
                          onClick={() => setWallpaper(path, targetMonitorId, true)}
                          className={`w-32 h-24 rounded-lg ${theme === 'light' ? 'bg-black/5 border-black/10' : 'bg-white/10 border-white/10'} border overflow-hidden cursor-pointer hover:border-blue-500 transition-colors hover:shadow-[0_0_15px_rgba(59,130,246,0.3)] relative group flex items-center justify-center`}
                        >
                          {historyBlobs[path] ? (
                            <img src={historyBlobs[path]} alt={`Wallpaper ${index}`} className="w-full h-full object-cover" />
                          ) : (
                            <ImageIcon size={24} className={`${theme === 'light' ? 'text-black/20' : 'text-white/20'}`} />
                          )}
                        </div>
                      ))}
                      <div 
                        onClick={() => setShowFilePicker(true)}
                        className={`w-32 h-24 rounded-lg border-2 border-dashed ${theme === 'light' ? 'border-black/20 hover:bg-black/5 hover:border-black/40' : 'border-white/20 hover:bg-white/5 hover:border-white/40'} flex flex-col items-center justify-center gap-2 cursor-pointer transition-colors`}
                      >
                        <Plus size={20} className={`${theme === 'light' ? 'text-black/60' : 'text-white/60'}`} />
                        <span className={`text-[12px] ${theme === 'light' ? 'text-black/60' : 'text-white/60'}`}>Browse</span>
                      </div>
                    </div>
                  </div>

                </>
              )}
              {activeTab === 'display' && (
                <>
                  <h3 className={`text-xl font-semibold mb-6 ${theme === 'light' ? 'text-black/90' : 'text-white/90'}`}>Display</h3>
                  <div className="flex flex-col gap-4 mb-8">
                    <span className={`text-[14px] ${theme === 'light' ? 'text-black/80' : 'text-white/80'}`}>Your displays</span>
                    <span className={`text-[13px] ${theme === 'light' ? 'text-black/50' : 'text-white/40'}`}>These are the displays currently connected to your system. Their arrangement is managed by your operating system.</span>
                  </div>

                  {/* Monitor Arrangement Area */}
                  <div className={`w-full h-[300px] ${theme === 'light' ? 'bg-[#ebebeb] border-black/10' : 'bg-[#0a0a0a] border-white/10'} border rounded-xl relative overflow-hidden flex items-center justify-center p-8 mb-8`}>
                    {!isMultiMonitor ? (
                      <div className={`flex flex-col items-center justify-center ${theme === 'light' ? 'text-black/40' : 'text-white/40'} gap-4`}>
                        <Monitor size={48} strokeWidth={1} />
                        <span className="text-sm">Only one display detected.</span>
                      </div>
                    ) : (
                      <div className="relative w-full h-full flex items-center justify-center">
                        {(() => {
                           const minX = Math.min(...displays.map(d => d.logicalX));
                           const minY = Math.min(...displays.map(d => d.logicalY));
                           const maxX = Math.max(...displays.map(d => d.logicalX + d.logicalWidth));
                           const maxY = Math.max(...displays.map(d => d.logicalY + d.logicalHeight));
                           const totalW = maxX - minX;
                           const totalH = maxY - minY;
                           
                           // Calcula a escala para caber dentro da div de 400x200 (com margem)
                           const scale = Math.min(400 / totalW, 200 / totalH);

                           return displays.map((d, i) => (
                            <div
                              key={d.id}
                              className={`absolute flex flex-col items-center justify-center rounded-lg border-2 shadow-lg transition-colors ${
                                d.isPrimary ? `bg-blue-600/20 border-blue-500 z-10 ${theme === 'light' ? 'text-black' : 'text-white'}` : `${theme === 'light' ? 'bg-black/5 border-black/20 text-black' : 'bg-white/5 border-white/20 text-white'} z-0`
                              }`}
                              style={{
                                width: d.logicalWidth * scale,
                                height: d.logicalHeight * scale,
                                left: '50%',
                                top: '50%',
                                marginLeft: ((d.logicalX - minX) - totalW/2) * scale,
                                marginTop: ((d.logicalY - minY) - totalH/2) * scale,
                              }}
                            >
                              <span className="text-2xl font-bold opacity-50">{i + 1}</span>
                              {d.isPrimary && <span className="text-[10px] mt-1 bg-blue-500 text-white px-2 py-0.5 rounded-full">Primary</span>}
                            </div>
                           ));
                        })()}
                      </div>
                    )}
                  </div>

                  <div className={`w-full h-[1px] ${theme === 'light' ? 'bg-black/5' : 'bg-white/5'} mb-8`}></div>
                  
                  <h3 className={`text-xl font-semibold mb-6 ${theme === 'light' ? 'text-black/90' : 'text-white/90'}`}>Multiple displays</h3>
                  <div className="flex flex-col gap-8 pb-10">
                    {displays.map((d) => (
                      <div key={d.id} className={`flex items-center justify-between gap-4 p-4 border ${theme === 'light' ? 'border-black/5 bg-[#f0f0f0]' : 'border-white/5 bg-[#1a1a1a]/50'} rounded-xl`}>
                        <div className="flex items-center gap-4">
                          <Monitor size={24} className={d.isPrimary ? "text-blue-500" : (theme === 'light' ? "text-black/40" : "text-white/40")} />
                          <div className="flex flex-col">
                            <span className={`text-[14px] font-semibold ${theme === 'light' ? 'text-black/90' : 'text-white/90'}`}>{d.name} {d.isPrimary ? '(Primary)' : ''}</span>
                            <span className={`text-[13px] ${theme === 'light' ? 'text-black/40' : 'text-white/40'} mt-1`}>{d.physicalWidth} x {d.physicalHeight}</span>
                          </div>
                        </div>
                        
                        {!d.isPrimary && (
                          <div 
                            onClick={() => updateDisplayLayout(d.id, { isPrimary: true })}
                            className="flex items-center gap-2 bg-blue-600 hover:bg-blue-500 px-4 py-2 rounded-lg cursor-pointer transition-colors shrink-0"
                          >
                            <span className="text-[13px] text-white font-medium">Make this my main display</span>
                          </div>
                        )}
                      </div>
                    ))}
                  </div>

                </>
              )}

            </div>
          </div>

        </div>
      </div>
      {/* File Picker Modal */}
      <AnimatePresence>
        {showFilePicker && (
          <CustomFilePicker 
            onClose={() => setShowFilePicker(false)}
            onSelect={(url, path) => {
              if (path) {
                setWallpaper(path, targetMonitorId, true); // Pass true to indicate it's a local path
              } else {
                setWallpaper(url, targetMonitorId);
              }
              setShowFilePicker(false);
            }}
          />
        )}
      </AnimatePresence>
    </div>
  );
};

/* --- Componentes Reutilizáveis Baseados no Design --- */

const IconBtn = ({ icon: Icon, active = false, theme = 'dark' }: any) => (
  <div className={`w-10 h-10 rounded-full flex items-center justify-center cursor-pointer transition-all ${
    active ? 'bg-blue-600/10 text-blue-500' : (theme === 'light' ? 'text-black/40 hover:bg-black/5 hover:text-black/80' : 'text-white/40 hover:bg-white/5 hover:text-white/80')
  }`}>
    <Icon size={20} />
  </div>
);

const NavSection = ({ title, children, theme = 'dark' }: any) => (
  <div className="flex flex-col gap-1">
    <span className={`text-[10px] font-bold ${theme === 'light' ? 'text-black/40' : 'text-white/30'} tracking-wider mb-2 px-3`}>{title}</span>
    {children}
  </div>
);

const NavItem = ({ icon: Icon, label, active = false, compact = false, onClick, theme = 'dark' }: any) => (
  <div onClick={onClick} className={`flex items-center gap-3 px-3 py-2 rounded-lg cursor-pointer transition-all ${
    active 
      ? (theme === 'light' ? 'bg-[#e0e0e0] text-black font-medium' : 'bg-[#1a1a1a] text-white font-medium')
      : (theme === 'light' ? 'text-black/60 hover:bg-black/5 hover:text-black/90' : 'text-white/60 hover:bg-white/5 hover:text-white/90')
  }`}>
    <Icon size={compact ? 16 : 18} className={active ? 'text-blue-500' : (theme === 'light' ? 'text-black/40' : 'text-white/40')} />
    <span className={`text-[13px] ${compact ? '' : 'mt-0.5'}`}>{label}</span>
  </div>
);

const CustomCheckbox = ({ label, checked, theme = 'dark' }: any) => (
  <label className="flex items-center gap-3 cursor-pointer group w-fit">
    <div className={`w-[18px] h-[18px] rounded-[4px] flex items-center justify-center transition-colors ${
      checked ? 'bg-blue-600' : `bg-transparent border ${theme === 'light' ? 'border-black/20 group-hover:border-black/40' : 'border-white/20 group-hover:border-white/40'}`
    }`}>
      {checked && <Check size={12} className="text-white" strokeWidth={3} />}
    </div>
    <span className={`text-[13px] ${theme === 'light' ? 'text-black/80 group-hover:text-black' : 'text-white/80 group-hover:text-white'} transition-colors`}>{label}</span>
  </label>
);

const ToggleRow = ({ title, desc, checked, onChange, theme = 'dark' }: any) => (
  <div className="flex items-center justify-between gap-4">
    <div className="flex flex-col">
      <span className={`text-[14px] font-semibold ${theme === 'light' ? 'text-black/90' : 'text-white/90'}`}>{title}</span>
      <span className={`text-[13px] ${theme === 'light' ? 'text-black/50' : 'text-white/40'} mt-1`}>{desc}</span>
    </div>
    <div 
      onClick={onChange}
      className={`w-11 h-6 rounded-full flex items-center p-1 cursor-pointer transition-colors shrink-0 ${
        checked ? 'bg-blue-600' : (theme === 'light' ? 'bg-black/10' : 'bg-white/10')
      }`}
    >
      <motion.div 
        layout
        className="w-4 h-4 bg-white rounded-full shadow-sm"
        animate={{ x: checked ? 20 : 0 }}
        transition={{ type: "spring", stiffness: 500, damping: 30 }}
      />
    </div>
  </div>
);

const DropdownRow = ({ title, desc, value, onClick, theme = 'dark' }: any) => (
  <div className="flex items-center justify-between gap-4">
    <div className="flex flex-col">
      <span className={`text-[14px] font-semibold ${theme === 'light' ? 'text-black/90' : 'text-white/90'}`}>{title}</span>
      <span className={`text-[13px] ${theme === 'light' ? 'text-black/50' : 'text-white/40'} mt-1`}>{desc}</span>
    </div>
    <div onClick={onClick} className={`flex items-center gap-2 ${theme === 'light' ? 'bg-[#ffffff] border-black/10 hover:bg-black/5' : 'bg-[#1a1a1a] border-white/10 hover:bg-white/5'} border px-3 py-1.5 rounded-lg cursor-pointer transition-colors shrink-0`}>
      <span className={`text-[13px] ${theme === 'light' ? 'text-black/80' : 'text-white/80'}`}>{value}</span>
      <ChevronDown size={14} className={`${theme === 'light' ? 'text-black/40' : 'text-white/40'}`} />
    </div>
  </div>
);

export default SettingsApp;

const CustomFilePicker = ({ onClose, onSelect }: { onClose: () => void, onSelect: (url: string, path?: string) => void }) => {
  const dragControls = useDragControls();

  return (
    <motion.div 
      className="absolute inset-0 z-[100] flex items-center justify-center pointer-events-none"
    >
      <motion.div
        drag
        dragListener={false}
        dragControls={dragControls}
        initial={{ opacity: 0, scale: 0.95 }}
        animate={{ opacity: 1, scale: 1 }}
        exit={{ opacity: 0, scale: 0.95 }}
        className="w-[850px] h-[550px] shadow-2xl rounded-xl border border-white/20 pointer-events-auto flex flex-col overflow-hidden bg-[#1e1e1e]"
      >
        {/* Title bar for drag */}
        <div onPointerDown={(e) => dragControls.start(e)} className="h-10 bg-black/80 flex items-center justify-between px-4 cursor-default select-none shrink-0 border-b border-white/10">
          <span className="text-xs font-semibold text-white/80">Select Wallpaper</span>
          <button onClick={onClose} className="w-6 h-6 flex items-center justify-center hover:bg-white/10 rounded-full transition-colors"><X size={14}/></button>
        </div>
        
        {/* The Explorer UI */}
        <div className="flex-1 overflow-hidden relative">
           <FileExplorerBase isPicker onFileSelect={onSelect} onClosePicker={onClose} />
        </div>
      </motion.div>
    </motion.div>
  );
};