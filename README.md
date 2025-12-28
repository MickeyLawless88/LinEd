# LinEd - Enhanced EDLIN-like Line Editor

## Overview

LinEd is a full-featured, EDLIN-compatible line editor designed for MS-DOS, and IBM-PC compatible systems. This enhanced version maintains full compatibility with the original EDLIN command set while adding modern conveniences and a professional startup experience.

## Key Features

### Enhanced Startup Banner
The original LinEd featured a simple 4-line banner. This version includes a **completely redesigned professional startup banner** featuring:
- Elegant ASCII art header with program name
- Version information and copyright notice
- System compatibility information
- **Real-time system clock display** (HH:MM:SS format)
- Active filename display in uppercase
- Professional formatting with visual separators

### Core Editor Features
- **Zero-padded line numbers**: Display format 00000, 00001, etc.
- **Case-insensitive commands**: Accept both uppercase and lowercase input
- **Multi-line insert mode**: Insert multiple lines with single command
- **Advanced search/replace**: Support for `/old/new/[g]` syntax with global option
- **Range parsing**: Flexible line range specifications (a,b format)
- **Status line**: Displays current file and line count after every command
- **Memory safety**: Robust memory management and error handling
- **Interactive prompts**: Each input line shows its current line number

## System Compatibility

- **MS-DOS**: Full compatibility with MS-DOS systems
- **Turbo C**: Compiled and tested with Turbo C compiler
- **IBM-PC Compatible**: Works on IBM-PC and compatible systems
- **Serial Terminals**: Optimized for teletype and serial console environments

## Command Reference

| Command | Syntax | Description |
|---------|--------|-------------|
| `L` | `L [a][,b]` | List lines (range optional) |
| `I` | `I [n]` | Insert at line n (end with single '.') |
| `D` | `D a[,b]` | Delete lines in range |
| `E` | `E n` | Edit (replace) single line |
| `R` | `R a[,b] /old/new/[g]` | Replace text; 'g' = global per line |
| `S` | `S [a][,b] /text/` | Search (case-insensitive) |
| `O` | `O name` | Open (load) file |
| `W` | `W [name]` | Write (save) file |
| `P` | `P` | Print status |
| `H` or `?` | `H` or `?` | Help |
| `Q` | `Q` | Quit |

## Usage Examples

### Basic File Operations
```
O myfile.txt    # Open file
L               # List all lines
I 5             # Insert at line 5
Hello World     # Type content
.               # End insert mode
W               # Save file
```

### Search and Replace
```
S /hello/       # Search for "hello"
R 1,10 /old/new/g  # Replace "old" with "new" globally in lines 1-10
```

### Range Operations
```
L 1,5           # List lines 1 through 5
D 3,7           # Delete lines 3 through 7
```

## Technical Specifications

- **Maximum lines**: 1,200 lines per file
- **Line length**: 256 characters maximum
- **Input buffer**: 512 characters
- **Memory management**: Dynamic allocation with safety checks
- **File format**: Standard text files with CR/LF line endings

## Compilation

### Turbo C (MS-DOS)
```
tcc -ml lined.c -o linednew.exe
```

### Modern C Compiler
```
gcc -o lined lined.c
```

## New Features in This Version

1. **Professional Startup Banner**: Complete redesign from original 4-line banner
2. **Real-time Clock**: System time display in banner (MS-DOS compatible)
3. **Enhanced Visual Design**: Professional formatting and layout
4. **Improved Status Display**: Comprehensive file and line information
5. **Better Error Handling**: More robust error messages and recovery
6. **Memory Safety**: Enhanced memory management and bounds checking

## File Structure

```
lined.c      # Main source file
README.md       # This documentation file
```

## Version History

- **Version 1.0**: Enhanced version with professional banner and real-time clock
- **Original**: Simple 4-line banner, basic functionality

## Copyright

Copyright (C) 2025-2026 M. Lawless

## License

This software is provided for educational and historical computing purposes. Compatible with vintage computing systems and modern development environments.

## Support

This editor is designed to work in resource-constrained environments typical of 1980s and early 1990s computing systems. It maintains compatibility with original EDLIN while providing enhanced user experience through improved visual design and real-time system information display.
