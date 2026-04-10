import React, { useState, useEffect } from 'react';
import { motion, AnimatePresence, useDragControls } from 'framer-motion';
import { 
  Search, Settings, Home, Users, Activity, Trophy, Calendar, Lightbulb, 
  FileSpreadsheet, Grid, Settings2, User, Sliders, AppWindow, ArrowUpCircle, 
  Shield, LayoutTemplate, CreditCard, Map, Check, ChevronDown, Plus, Zap, Layers, TreePine, Hexagon, ChevronRight,
  Folder, File as FileIcon, Image as ImageIcon, X, ChevronLeft, HardDrive
} from 'lucide-react';
import { invoke } from '@tauri-apps/api/core';

import { useTheme } from './ThemeContext';

import { FileExplorerBase } from './FileExplorer';

const SettingsApp = () => {
  const { theme, setTheme, setWallpaper } = useTheme();
  
  const [mobilePush, setMobilePush] = useState(true);
  const [desktopPush, setDesktopPush] = useState(true);
  const [emailPush, setEmailPush] = useState(false);
  const [twoFactor, setTwoFactor] = useState(true);
  const [showFilePicker, setShowFilePicker] = useState(false);

  return (
    <div className="flex w-full h-full bg-[#121212] text-white/90 overflow-hidden rounded-b-xl select-none font-sans border border-white/5">
      
      {/* 1. Leftmost Mini Sidebar (Brand/Tools) */}
      <div className="w-[60px] h-full bg-[#0a0a0a] border-r border-white/5 flex flex-col items-center py-6 gap-6 shrink-0 z-20">
        <div className="w-10 h-10 rounded-xl bg-blue-600/20 text-blue-500 flex items-center justify-center cursor-pointer hover:bg-blue-600/30 transition-colors">
          <Hexagon size={22} fill="currentColor" />
        </div>
        <div className="flex flex-col gap-4 mt-4">
          <IconBtn icon={Zap} active />
          <IconBtn icon={Layers} />
          <IconBtn icon={TreePine} />
        </div>
        <div className="mt-auto">
          <div className="w-10 h-10 rounded-full border border-white/10 text-white/40 flex items-center justify-center cursor-pointer hover:bg-white/5 transition-colors">
            <Plus size={20} />
          </div>
        </div>
      </div>

      {/* 2. Secondary Sidebar (Main Navigation) */}
      <div className="w-[240px] h-full bg-[#0f0f0f] border-r border-white/5 flex flex-col shrink-0 z-10 hidden md:flex">
        {/* User Profile Area */}
        <div className="h-[72px] flex items-center px-5 border-b border-white/5 gap-3 cursor-pointer hover:bg-white/5 transition-colors">
          <img src="https://i.pravatar.cc/150?img=11" alt="User" className="w-10 h-10 rounded-full border border-white/10" />
          <div className="flex flex-col overflow-hidden">
            <span className="font-semibold text-[14px] truncate">Rafiqur...</span>
            <span className="text-[11px] text-white/40 truncate">rafiqur51@jira.com</span>
          </div>
          <Settings size={16} className="text-white/40 ml-auto" />
        </div>

        {/* Navigation Links */}
        <div className="flex-1 overflow-y-auto custom-scrollbar px-3 py-5 flex flex-col gap-6">
          
          <NavSection title="ANALYTICS">
            <NavItem icon={Home} label="Overview" />
            <NavItem icon={Users} label="Team Insights" />
            <NavItem icon={Activity} label="Engagement" />
            <NavItem icon={Trophy} label="Leaderboard" />
          </NavSection>

          <NavSection title="CONTEXT">
            <NavItem icon={Calendar} label="Calendar Events" />
            <NavItem icon={Lightbulb} label="Insights" />
            <NavItem icon={FileSpreadsheet} label="Spreadsheet" />
          </NavSection>

          <NavSection title="OTHERS">
            <NavItem icon={Grid} label="Apps" />
            <NavItem icon={Settings2} label="Properties" />
            <NavItem icon={Settings} label="Settings" active />
          </NavSection>

        </div>
      </div>

      {/* 3. Main Content Area (Topbar + Content) */}
      <div className="flex-1 flex flex-col h-full bg-[#121212] overflow-hidden">
        
        {/* Topbar */}
        <div className="h-[72px] border-b border-white/5 flex items-center px-8 justify-between shrink-0 bg-[#0f0f0f]/50 backdrop-blur-md">
          {/* Search */}
          <div className="relative flex items-center bg-[#1a1a1a] rounded-lg px-3 py-2 w-[300px] border border-white/5 focus-within:border-blue-500/50 transition-colors">
            <Search size={16} className="text-white/40 mr-2" />
            <input 
              type="text" 
              placeholder="Search item" 
              className="bg-transparent border-none outline-none w-full text-[13px] placeholder:text-white/40 text-white"
            />
            <span className="text-[10px] text-white/30 border border-white/10 px-1.5 py-0.5 rounded ml-auto">⌘K</span>
          </div>

          {/* Right Area (Profile without bell) */}
          <div className="flex items-center gap-4">
            <img src="https://i.pravatar.cc/150?img=11" alt="Profile" className="w-8 h-8 rounded-full border border-white/10 cursor-pointer hover:opacity-80 transition-opacity" />
          </div>
        </div>

        {/* Content Body (Third Sidebar + Actual Settings) */}
        <div className="flex-1 flex h-[calc(100%-72px)]">
          
          {/* Third Sidebar (Sub-navigation) */}
          <div className="w-[240px] h-full border-r border-white/5 flex flex-col shrink-0 bg-[#121212] hidden lg:flex">
            <div className="p-8 pb-4">
              <h2 className="text-[22px] font-semibold flex items-center gap-2">
                Settings <ChevronRight size={16} className="text-white/40" /> <span className="text-white/60 text-[15px] font-normal">General</span>
              </h2>
            </div>
            <div className="flex-1 overflow-y-auto custom-scrollbar px-5 pb-6 flex flex-col gap-6">
              
              <NavSection title="ACCOUNT">
                <NavItem icon={User} label="My Profile" compact />
                <NavItem icon={Home} label="General" active compact />
                <NavItem icon={Sliders} label="Preferences" compact />
                <NavItem icon={AppWindow} label="Applications" compact />
              </NavSection>

              <NavSection title="WORKSPACE">
                <NavItem icon={Settings} label="Settings" compact />
                <NavItem icon={Users} label="Members" compact />
                <NavItem icon={ArrowUpCircle} label="Upgrade" compact />
                <NavItem icon={Shield} label="Security" compact />
                <NavItem icon={LayoutTemplate} label="Templates" compact />
                <NavItem icon={CreditCard} label="Billing" compact />
                <NavItem icon={Map} label="Roadmaps" compact />
              </NavSection>

            </div>
          </div>

          {/* Actual Settings Form */}
          <div className="flex-1 h-full overflow-y-auto custom-scrollbar p-8 lg:p-12">
            <div className="max-w-3xl">
              
              {/* Notifications Section */}
              <h3 className="text-xl font-semibold mb-6 text-white/90">My Notifications</h3>
              
              <div className="flex justify-between items-center mb-4">
                <span className="text-[14px] font-semibold">Notify me when...</span>
                <span className="text-[13px] text-blue-500 cursor-pointer hover:underline">About notifications?</span>
              </div>
              
              <div className="flex flex-col gap-3 mb-10">
                <CustomCheckbox label="Daily productivity update" checked />
                <CustomCheckbox label="New event created" checked />
                <CustomCheckbox label="When added on new team" checked />
              </div>

              <div className="flex flex-col gap-8 mb-12">
                <ToggleRow 
                  title="Mobile push notifications" 
                  desc="Receive push notification whenever your organisation requires your attentions" 
                  checked={mobilePush} onChange={() => setMobilePush(!mobilePush)} 
                />
                <ToggleRow 
                  title="Desktop Notification" 
                  desc="Receive desktop notification whenever your organisation requires your attentions" 
                  checked={desktopPush} onChange={() => setDesktopPush(!desktopPush)} 
                />
                <ToggleRow 
                  title="Email Notification" 
                  desc="Receive email whenever your organisation requires your attentions" 
                  checked={emailPush} onChange={() => setEmailPush(!emailPush)} 
                />
              </div>

              {/* Settings Section */}
              <div className="w-full h-[1px] bg-white/5 mb-8"></div>
              <h3 className="text-xl font-semibold mb-6 text-white/90">My Settings</h3>

              <div className="flex flex-col gap-8 pb-10">
                <DropdownRow 
                  title="Appearance" 
                  desc="Customize how the theme looks on your device." 
                  value={theme === 'dark' ? 'Dark' : 'Light'}
                  onClick={() => setTheme(theme === 'dark' ? 'light' : 'dark')}
                />
                
                <div className="flex items-center justify-between gap-4">
                  <div className="flex flex-col">
                    <span className="text-[14px] font-semibold text-white/90">System Wallpaper</span>
                    <span className="text-[13px] text-white/40 mt-1">Choose a background image from your files.</span>
                  </div>
                  <div 
                    onClick={() => setShowFilePicker(true)}
                    className="flex items-center gap-2 bg-[#1a1a1a] border border-white/10 px-4 py-2 rounded-lg cursor-pointer hover:bg-white/5 transition-colors shrink-0"
                  >
                    <span className="text-[13px] text-white/80">Browse files</span>
                  </div>
                </div>

                <ToggleRow 
                  title="Two-factor authentication" 
                  desc="Keep your account secure by enabling 2FA via SMS or using a temporary one-time passcode (TOTP)." 
                  checked={twoFactor} onChange={() => setTwoFactor(!twoFactor)} 
                />
                <DropdownRow 
                  title="Language" 
                  desc="Customize the language of the system." 
                  value="English" 
                />
              </div>

            </div>
          </div>

        </div>
      </div>
      {/* File Picker Modal */}
      <AnimatePresence>
        {showFilePicker && (
          <CustomFilePicker 
            onClose={() => setShowFilePicker(false)}
            onSelect={(url) => {
              setWallpaper(url);
              setShowFilePicker(false);
            }}
          />
        )}
      </AnimatePresence>
    </div>
  );
};

/* --- Componentes Reutilizáveis Baseados no Design --- */

const IconBtn = ({ icon: Icon, active = false }) => (
  <div className={`w-10 h-10 rounded-full flex items-center justify-center cursor-pointer transition-all ${
    active ? 'bg-blue-600/10 text-blue-500' : 'text-white/40 hover:bg-white/5 hover:text-white/80'
  }`}>
    <Icon size={20} />
  </div>
);

const NavSection = ({ title, children }) => (
  <div className="flex flex-col gap-1">
    <span className="text-[10px] font-bold text-white/30 tracking-wider mb-2 px-3">{title}</span>
    {children}
  </div>
);

const NavItem = ({ icon: Icon, label, active = false, compact = false }) => (
  <div className={`flex items-center gap-3 px-3 py-2 rounded-lg cursor-pointer transition-all ${
    active 
      ? 'bg-[#1a1a1a] text-white font-medium' 
      : 'text-white/60 hover:bg-white/5 hover:text-white/90'
  }`}>
    <Icon size={compact ? 16 : 18} className={active ? 'text-blue-500' : 'text-white/40'} />
    <span className={`text-[13px] ${compact ? '' : 'mt-0.5'}`}>{label}</span>
  </div>
);

const CustomCheckbox = ({ label, checked }) => (
  <label className="flex items-center gap-3 cursor-pointer group w-fit">
    <div className={`w-[18px] h-[18px] rounded-[4px] flex items-center justify-center transition-colors ${
      checked ? 'bg-blue-600' : 'bg-transparent border border-white/20 group-hover:border-white/40'
    }`}>
      {checked && <Check size={12} className="text-white" strokeWidth={3} />}
    </div>
    <span className="text-[13px] text-white/80 group-hover:text-white transition-colors">{label}</span>
  </label>
);

const ToggleRow = ({ title, desc, checked, onChange }) => (
  <div className="flex items-center justify-between gap-4">
    <div className="flex flex-col">
      <span className="text-[14px] font-semibold text-white/90">{title}</span>
      <span className="text-[13px] text-white/40 mt-1">{desc}</span>
    </div>
    <div 
      onClick={onChange}
      className={`w-11 h-6 rounded-full flex items-center p-1 cursor-pointer transition-colors shrink-0 ${
        checked ? 'bg-blue-600' : 'bg-white/10'
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

const DropdownRow = ({ title, desc, value, onClick }: any) => (
  <div className="flex items-center justify-between gap-4">
    <div className="flex flex-col">
      <span className="text-[14px] font-semibold text-white/90">{title}</span>
      <span className="text-[13px] text-white/40 mt-1">{desc}</span>
    </div>
    <div onClick={onClick} className="flex items-center gap-2 bg-[#1a1a1a] border border-white/10 px-3 py-1.5 rounded-lg cursor-pointer hover:bg-white/5 transition-colors shrink-0">
      <span className="text-[13px] text-white/80">{value}</span>
      <ChevronDown size={14} className="text-white/40" />
    </div>
  </div>
);

export default SettingsApp;

const CustomFilePicker = ({ onClose, onSelect }: { onClose: () => void, onSelect: (url: string) => void }) => {
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