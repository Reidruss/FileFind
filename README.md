# FileFind

FileFind is a lightweight file manager built with C and GTK4 for Linux.

## Current Features

- Navigate directories by typing a path in the top input and pressing Enter.
- Browse common locations from the sidebar:
  - Home
  - Desktop
  - Documents
  - Music
  - Pictures
  - Videos
  - Trash
- Open files with the system default application.
- Create files from the `Create File` button.
- Right-click file items for context actions:
  - Open
  - Rename
  - Delete
- Double-click (activate) list items:
  - Directories: enter directory
  - Files: open with default application

## Tech Stack

- C
- GTK4
- POSIX/GLib file APIs

## Prerequisites

- GCC
- `make`
- GTK4 development libraries

Ubuntu/Debian:

```bash
sudo apt update
sudo apt install -y build-essential libgtk-4-dev
```

## Build and Run

From the project root:

```bash
make
./main
```

## Project Structure

```text
include/
  app.h
  gui.h
  ops.h
src/
  app.c
  gui.c
  main.c
  ops.c
```

## Notes

- Context menu actions are currently shown only for file entries (not folders).
- Hidden entries (dotfiles) are not displayed in the file list.
