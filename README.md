# Scripturae

A repository for storing and developing random scripts and tools.

## Repository Structure

Each project should be in its own directory with a README describing its purpose and usage.

```
Scripturae/
├── README.md (this file)
├── project-name-1/
│   ├── README.md
│   └── ...
└── project-name-2/
    ├── README.md
    └── ...
```

## Guidelines

### Project Organization
- **One directory per project** - Keep each script/tool in its own folder
- **README required** - Each project must have a README.md with:
  - Brief description of what it does
  - Installation instructions (dependencies, requirements)
  - Usage examples
  - Any known issues or limitations

### Troubleshooting & Documentation
When you encounter problems:
1. Document the issue in this section
2. Document the solution that worked
3. This helps future users (including yourself and AI assistants)

## Known Issues & Solutions

### Python Version Conflicts
**Problem**: Scripts may fail when dependencies are installed for one Python version but executed with another.

**Example**: `pip install` installed packages to Python 3.10, but script ran with Python 3.13.

**Solutions**:
- Use virtual environments for each project:
  ```bash
  cd project-name
  python3 -m venv venv
  source venv/bin/activate  # On Linux/Mac
  # or
  venv\Scripts\activate  # On Windows
  pip install -r requirements.txt
  ```
- Check your Python version before running:
  ```bash
  python --version
  python3 --version
  ```
- Use specific Python version explicitly:
  ```bash
  python3.13 script.py
  ```
- Consider using `pyenv` to manage Python versions

## Projects

- [abb-rapid-text-generator](./abb-rapid-text-generator/) - Generate ABB RAPID code for drawing text with a robot
- [esp32-wifi-config-template](./esp32-wifi-config-template/) - WiFi Access Point with web-based configuration dashboard for ESP32 projects
