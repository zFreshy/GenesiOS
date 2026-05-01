---
name: Bug Report
about: Report a bug to help us improve Genesi OS
title: '[BUG] '
labels: bug
assignees: ''
---

## 🐛 Bug Description

A clear and concise description of what the bug is.

## 📋 To Reproduce

Steps to reproduce the behavior:
1. Go to '...'
2. Click on '...'
3. Scroll down to '...'
4. See error

## ✅ Expected Behavior

A clear and concise description of what you expected to happen.

## 📸 Screenshots

If applicable, add screenshots to help explain your problem.

## 💻 System Information

- **Genesi OS Version**: [e.g. 2026.05.01]
- **Kernel**: [run `uname -r`]
- **Desktop Environment**: [e.g. KDE Plasma 6.0]
- **CPU**: [e.g. Intel i7-12700K]
- **RAM**: [e.g. 16GB]
- **GPU**: [e.g. NVIDIA RTX 3060 / Integrated]
- **Installation Type**: [Live ISO / Installed to disk]

## 📝 Logs

<details>
<summary>System Logs</summary>

```bash
# Run these commands and paste output:
journalctl -b -p err
dmesg | tail -50
```

</details>

<details>
<summary>AI Mode Logs (if related)</summary>

```bash
# Run these commands and paste output:
sudo systemctl status genesi-aid
sudo journalctl -u genesi-aid -n 50
cat /var/run/genesi-aid.state
```

</details>

## 🔍 Additional Context

Add any other context about the problem here.

## ✔️ Checklist

- [ ] I have searched for similar issues
- [ ] I am using the latest version of Genesi OS
- [ ] I have included system information
- [ ] I have included relevant logs
