/** @type {import('tailwindcss').Config} */
export default {
  content: [
    "./index.html",
    "./src/**/*.{js,ts,jsx,tsx}",
  ],
  theme: {
    extend: {
      colors: {
        'glass-bg': 'rgba(30, 30, 30, 0.4)',
        'glass-border': 'rgba(255, 255, 255, 0.1)',
        'genesi-blue': '#007aff',
        'genesi-yellow': '#f39c12',
      }
    },
  },
  plugins: [],
}

