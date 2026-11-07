# MikasViewer

MikasViewer is a lightweight Windows image viewer for browsing, zooming, and printing common image formats. It is built with MFC and GDI+.

## Screenshot

![MikasViewer](docs/images/2026-05-19-MikasViewer-Screenshot.png)

## Overview

MikasViewer is designed for users who want an immediate, no-frills workflow:

- Explore drives and folders through an Explorer-style tree,
- Open images directly from the tree without a separate file dialog,
- View and zoom images quickly with smooth GDI+ rendering,
- Pan large images using drag or the scroll bars,
- Print or preview the current image in a single click.

## Audience

This application is intended for:

- Desktop users who browse folders of photographs and illustrations,
- Windows users who prefer a simple native MFC viewer,
- Developers or testers who need a minimal reference app for image viewing and printing.

## Features

- Explorer-style folder tree with logical drives at the root,
- Lazy loading for large directory structures,
- Image files displayed inline with folders,
- Manage mode with a folder tree, image thumbnail list, and image preview,
- View mode for inspecting the selected image without the browsing panes,
- Previous, next, first, and last image navigation in the current folder,
- Slideshow of the current folder with configurable interval and repeat,
- Fit-to-window rendering with centered presentation,
- Mouse-wheel zoom anchored to the cursor position,
- Drag-to-pan behavior for large images,
- Double-buffered drawing for a smooth, low-flicker viewer experience,
- Dark viewing background similar to common image viewer apps,
- Window title bar showing the current filename,
- WebP decoding through WIC when GDI+ cannot open the file,
- Print and print-preview support with page scaling,
- Built-in CHM help via F1 or Help > Help Topics.

## Supported Image Formats

| Extension | Format |
|-----------|--------|
| `.jpg` / `.jpeg` | JPEG |
| `.png` | PNG |
| `.bmp` | Bitmap |
| `.gif` | GIF |
| `.tif` / `.tiff` | TIFF |
| `.webp` | WebP (via WIC) |

> WebP support on Windows 10 requires the [WebP Image Extensions](https://apps.microsoft.com/detail/9pg2dk419drg) app from the Microsoft Store. On Windows 11, the codec is built in.

## Requirements

- Windows 10 or later,
- Visual Studio 2026 Community with the v145 C++ toolset,
- MFC support from the "Desktop development with C++" workload,
- Optional: HTML Help Workshop for rebuilding the CHM help file,
- Optional: Inno Setup 6 for creating the installer package.

## Build Instructions

1. Clone the repository with its GoogleTest submodule:

   ```powershell
   git clone --recurse-submodules https://github.com/mphuttu/MikasViewer.git
   ```

   For an existing clone, run `git submodule update --init --recursive` from the repository root.

2. Open `MikasViewer.slnx` in Visual Studio.
3. Set the configuration to **Debug** or **Release** and the platform to **x64**.
4. Choose **Build > Build Solution**.
5. Run `x64\Debug\MikasViewerTests.exe` or `x64\Release\MikasViewerTests.exe` to execute the tests.

The build output is placed in `x64\Debug\` or `x64\Release\` depending on the selected configuration.

> Help file rebuild in PowerShell: `& "C:\Program Files (x86)\HTML Help Workshop\hhc.exe" help\MikasViewer.hhp`

## Documentation

- Help topics: `help\`
- CHM help file: `help\MikasViewer.chm`
- Main project docs: `README.md`

## Creating the Installer

The installer is created with [Inno Setup 6](https://jrsoftware.org/isinfo.php).

Prerequisites:

1. Build the **Release x64** configuration first.
2. Rebuild the CHM help file so `help\MikasViewer.chm` exists.
3. Install [Inno Setup 6](https://jrsoftware.org/isdl.php) if not already available.

Build command:

```powershell
& "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" MikasViewer.iss
```

The resulting package is written to `setup_output\MikasViewerSetup-1.0-x64.exe`.

## Usage

| Action | Result |
|--------|--------|
| Click **Manage** / **View** | Switch between browsing and image viewing |
| Click a folder in the tree | Lists supported images and previews the first image by name |
| Click an image file in the tree or a thumbnail | Displays that image |
| Double-click a thumbnail | Opens View mode |
| Left / Right, Home / End | Previous / next, first / last image when the image or thumbnail list has focus |
| Click an arrow beside the image | Open the previous or next image when space permits |
| Slideshow > Start | Start from the selected image in View mode |
| Slideshow > Stop or `Esc` | Stop the slideshow; the current image remains visible |
| Mouse wheel | Zoom in / out, anchored to the cursor |
| Left-click + drag | Pan the image |
| `Ctrl+P` | Print the current image |
| `F1` or Help > Help Topics | Open the built-in help file |

The Slideshow menu offers intervals of 2, 5, 10, or 30 seconds; the default is
5 seconds. Repeat is enabled by default and can be turned off to stop at the
last image. Settings last until the application closes. Switching to Manage mode,
printing, or an image-loading error also stops the slideshow. The folder tree
keeps its normal keyboard navigation when it has focus.

## Troubleshooting

- Help does not open: ensure `MikasViewer.chm` exists next to the executable.
- Images do not display: verify the file is a supported format and that the file extension matches the content.
- WebP files fail to load: install the WebP Image Extensions app on Windows 10.
- Print is unavailable: confirm that an image is currently displayed.
- A thumbnail is missing: Windows could not provide a thumbnail for that file; try selecting the filename to open it.
- The application does not start: install the Visual C++ 2015–2022 x64 redistributable if required.

## Contributing

Contributions are welcome. Please:

- Keep changes focused and compatible with Windows MFC conventions,
- Preserve existing public file formats, keyboard mappings, and backward compatibility,
- Validate with a clean build before submitting,
- Update the help documentation when user-visible behavior changes,
- Keep commit messages concise and meaningful.

## License

This project is licensed under the MIT license.
