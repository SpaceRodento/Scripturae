# ABB RAPID Text Generator

Python script that generates ABB RAPID code for drawing text with a robot. The script converts TrueType fonts to robot movement paths.

## Features

- Convert any text to ABB RAPID robot code
- **Three bundled fonts ready to use** - no need to find font files!
- Support for custom TrueType (.ttf) fonts
- Adjustable text size in millimeters
- Configurable point density for smooth curves
- Custom WorkObject support
- Generates ready-to-use .mod files for RobotStudio
- Interactive or command-line mode
- Automatic bezier curve linearization

## Bundled Fonts

The script includes three ready-to-use fonts (DejaVu font family):

- **sans** - DejaVu Sans: Clean, general-purpose font for regular text
- **bold** - DejaVu Sans Bold: Thick, prominent font perfect for titles and headers
- **mono** - DejaVu Sans Mono: Monospace/typewriter style font for a technical look

No need to search for font files - just use `--font sans`, `--font bold`, or `--font mono`!

## Installation

### Prerequisites

- Python 3.7 or higher
- pip (Python package manager)

### Install Dependencies

**Important**: Use a virtual environment to avoid Python version conflicts!

```bash
# Navigate to project directory
cd abb-rapid-text-generator

# Create virtual environment
python3 -m venv venv

# Activate virtual environment
# On Linux/Mac:
source venv/bin/activate
# On Windows:
# venv\Scripts\activate

# Install dependencies
pip install -r requirements.txt
```

### Verify Installation

```bash
python rapid_text_generator.py --help
```

## Usage

### Interactive Mode

Simply run the script without arguments:

```bash
python rapid_text_generator.py
```

You'll be prompted to choose:
1. Text to draw
2. Font selection:
   - 1 = sans (DejaVu Sans - clean, regular)
   - 2 = bold (DejaVu Sans Bold - thick, prominent)
   - 3 = mono (DejaVu Sans Mono - monospace)
   - 4 = Custom path to .ttf file
3. Text height in mm
4. Style (outline or filled)
5. Point density in mm
6. WorkObject name
7. Output filename

### Command-Line Mode

Provide all parameters as arguments:

```bash
# Using bundled fonts (easiest!)
python rapid_text_generator.py \
  --text "HELLO ROBOT" \
  --font sans \
  --size 100 \
  --output hello.mod

# Using custom font file
python rapid_text_generator.py \
  --text "CUSTOM" \
  --font /path/to/your/font.ttf \
  --size 100 \
  --output custom.mod
```

### Parameters

- `--text`: Text to draw (required in CLI mode)
- `--font`: Font name (sans/bold/mono) or path to .ttf file (required in CLI mode)
- `--size`: Text height in millimeters (required in CLI mode)
- `--style`: Drawing style - `outline` or `filled` (default: `outline`)
- `--density`: Maximum distance between points on curves in mm (default: `2.0`)
  - Lower values = smoother curves but more points and slower execution
  - Higher values = faster execution but less smooth curves
- `--workobject`: WorkObject name for RAPID code (default: `wobj0`)
- `--output`: Output .mod filename (default: auto-generated from text)

## Examples

### Example 1: Regular Text with Sans Font

```bash
python rapid_text_generator.py \
  --text "HELLO" \
  --font sans \
  --size 100 \
  --output hello.mod
```

### Example 2: Title with Bold Font

```bash
python rapid_text_generator.py \
  --text "FACTORY 4.0" \
  --font bold \
  --size 200 \
  --output title.mod
```

### Example 3: Technical Text with Mono Font

```bash
python rapid_text_generator.py \
  --text "ROBOT-001" \
  --font mono \
  --size 80 \
  --output robot_id.mod
```

### Example 4: High Precision Text

```bash
python rapid_text_generator.py \
  --text "PRECISION" \
  --font sans \
  --size 150 \
  --density 1.0 \
  --workobject wobj1 \
  --output precision.mod
```

### Example 5: Fast Drawing with Lower Precision

```bash
python rapid_text_generator.py \
  --text "FAST" \
  --font bold \
  --size 80 \
  --density 5.0 \
  --output fast.mod
```

### Example 6: Using Custom Font

```bash
python rapid_text_generator.py \
  --text "CUSTOM STYLE" \
  --font /usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf \
  --size 100 \
  --output custom.mod
```

## Using Custom Fonts

While the bundled fonts cover most use cases, you can still use any TrueType font:

### Linux

Common font locations:
- `/usr/share/fonts/truetype/`
- `/usr/local/share/fonts/`
- `~/.fonts/`

List available fonts:
```bash
fc-list | grep -i "ttf"
```

### Windows

Common font locations:
- `C:\Windows\Fonts\`

### macOS

Common font locations:
- `/System/Library/Fonts/`
- `/Library/Fonts/`
- `~/Library/Fonts/`

## Using Generated Code in RobotStudio

1. **Open RobotStudio**
   - Start RobotStudio and open your robot project

2. **Load Module**
   - Go to `RAPID` tab
   - Right-click on the robot controller
   - Select `Load Module`
   - Browse and select the generated `.mod` file

3. **Verify Code**
   - Check that all robtargets are defined correctly
   - Verify WorkObject matches your setup

4. **Setup WorkObject**
   - Ensure the WorkObject (e.g., `wobj0`) is properly calibrated
   - The generated coordinates assume:
     - X: horizontal (text direction)
     - Y: vertical (text height)
     - Z: perpendicular to drawing surface (pen up/down)

5. **Configure Tool**
   - Ensure `tool0` (or your custom tool) is properly calibrated
   - The tool should have the pen/marker at the TCP (Tool Center Point)

6. **Test Run**
   - Move robot to a safe starting position
   - Run the `DrawText` procedure
   - Use slow speed first to verify paths

7. **Adjust Z-Height**
   - You may need to adjust the Z-offset in the code
   - Default pen lift is 10mm (modify in script if needed)
   - Ensure drawing surface is at Z=0 of the WorkObject

## Output Format

The generated `.mod` file contains:

1. **Header Comments**
   - Original text
   - Font used
   - Generation parameters
   - Statistics (total points, estimated time)

2. **robtarget Definitions**
   - One robtarget per point
   - Format: `p0`, `p1`, `p2`, etc.
   - Coordinates in mm relative to WorkObject

3. **DrawText Procedure**
   - Movement commands using `MoveL`
   - Pen lift/lower movements (Z-offset)
   - `fine` positioning for corners
   - `z1` zone for smooth curves

## Technical Details

### Coordinate System

- **X-axis**: Horizontal (left to right)
- **Y-axis**: Vertical (bottom to top)
- **Z-axis**: Perpendicular to drawing plane
  - Z=0: Pen touching surface
  - Z=10mm (default): Pen lifted

### Movement Strategy

- Robot moves to start position with pen lifted (+Z offset)
- Lowers pen to drawing surface (Z=0)
- Draws path using `MoveL` with fine positioning at endpoints
- Lifts pen between disconnected paths
- Uses `z1` zone for smooth motion along curves
- Uses `fine` positioning at path endpoints for accuracy

### Font Processing

- Extracts glyph outlines from TrueType fonts
- Converts bezier curves (quadratic and cubic) to line segments
- Adjusts point density based on curve length
- Scales font units to millimeters

## Troubleshooting

### "Module fonttools not found"

Ensure you've activated the virtual environment and installed dependencies:
```bash
source venv/bin/activate
pip install -r requirements.txt
```

### "Font file not found"

- Check the font path is correct
- Use absolute paths if relative paths don't work
- Verify the font is a TrueType (.ttf) file

### "Character not found in font"

- Some fonts don't include all characters
- Try a different font (e.g., DejaVuSans, Liberation, Arial)
- Check for special characters that might not be supported

### Generated paths are jagged

- Decrease the `--density` parameter (e.g., from 2.0 to 1.0)
- This creates more points for smoother curves
- Note: More points = longer execution time

### Generated paths are too detailed/slow

- Increase the `--density` parameter (e.g., from 2.0 to 5.0)
- This creates fewer points
- Trade-off: Less smooth curves

### Robot moves incorrectly in RobotStudio

- Verify WorkObject calibration
- Check that coordinate system matches expectations
- Ensure tool calibration is correct
- Verify Z=0 is at the drawing surface

### Python version conflicts

If you see errors about missing packages even after installation:

```bash
# Check which Python version you're using
python --version
python3 --version

# Create virtual environment with specific version
python3.13 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
```

## Limitations

- Currently only supports outline style (filled style draws outlines)
- No automatic scaling to fit specific dimensions
- No text alignment options (left/center/right)
- No multi-line text support
- Generated code uses `tool0` (can be modified in script)

## Future Enhancements

- True filled text support (hatching/zigzag patterns)
- Text alignment options
- Multi-line text with line spacing
- Automatic scaling to fit bounding box
- Different drawing patterns (spiral, zigzag)
- Optimization to reduce number of movements
- Support for other font formats

## License

Free to use and modify.

## Contributing

Feel free to submit issues or improvements to the repository.
