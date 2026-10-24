# Contributing to TinyAgent

Thanks for your interest in contributing! 🎉

## How to Contribute

### Bug Reports & Feature Requests
- Use [GitHub Issues](https://github.com/quantumnic/TinyAgent/issues)
- Check existing issues before creating a new one
- Use the issue templates provided

### Code Contributions

1. **Fork** the repository
2. **Create a branch**: `git checkout -b feature/my-feature`
3. **Make your changes** with clear, descriptive commits
4. **Test** on real ESP32 hardware if possible
5. **Submit a Pull Request**

### Coding Style

- Use 4-space indentation
- Follow Arduino/C++ naming conventions
- Prefix private members with `_`
- Keep memory usage minimal — avoid dynamic allocations where possible
- Add documentation comments for public APIs

### Adding a New Tool

1. Create `src/tools/YourTool.h`
2. Implement a static `registerAll(ToolRegistry& registry)` method
3. Register it in `ToolRegistry::registerBuiltins()` (or document as optional)
4. Add an example if appropriate
5. Update README.md tool table

### Commit Messages

Use clear, descriptive commit messages:
```
feat: add BLE tool for Bluetooth scanning
fix: handle timeout in LLM client
docs: update Quick Start guide
```

## Development Setup

1. Install [PlatformIO](https://platformio.org/)
2. Clone the repo
3. Open in VS Code with PlatformIO extension
4. Build examples to verify changes compile

## License

By contributing, you agree that your contributions will be licensed under the MIT License.
