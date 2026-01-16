#!/usr/bin/env python3
"""
ABB RAPID Text Generator

Generates ABB RAPID code for drawing text with a robot.

Example usage:
    # Interactive mode
    python rapid_text_generator.py

    # Command-line mode with bundled fonts
    python rapid_text_generator.py --text "HELLO" --font sans --size 100 --output hello.mod
    python rapid_text_generator.py --text "TITLE" --font bold --size 150 --output title.mod
    python rapid_text_generator.py --text "CODE" --font mono --size 80 --output code.mod

    # Or with custom font file
    python rapid_text_generator.py --text "CUSTOM" --font /path/to/font.ttf --size 100 --output custom.mod

Requirements:
    - fonttools
    - Pillow (PIL)

Bundled Fonts:
    - sans: DejaVu Sans (clean, general-purpose)
    - bold: DejaVu Sans Bold (thick, for titles)
    - mono: DejaVu Sans Mono (monospace, typewriter style)
"""

import argparse
import sys
import os
from pathlib import Path
from typing import List, Tuple
import math

try:
    from fontTools.ttLib import TTFont
    from fontTools.pens.recordingPen import RecordingPen
    from PIL import Image, ImageDraw, ImageFont
except ImportError as e:
    print(f"Error: Missing required library: {e}")
    print("Install dependencies with: pip install fonttools pillow")
    sys.exit(1)


# Bundled fonts
SCRIPT_DIR = Path(__file__).parent
FONTS_DIR = SCRIPT_DIR / "fonts"

BUNDLED_FONTS = {
    'sans': FONTS_DIR / 'DejaVuSans.ttf',
    'bold': FONTS_DIR / 'DejaVuSans-Bold.ttf',
    'mono': FONTS_DIR / 'DejaVuSansMono.ttf',
}


def resolve_font_path(font_name: str) -> Path:
    """
    Resolve font name to actual font file path.

    Args:
        font_name: Either a bundled font name (sans/bold/mono) or path to .ttf file

    Returns:
        Path to font file

    Raises:
        ValueError if font not found
    """
    # Check if it's a bundled font name
    if font_name.lower() in BUNDLED_FONTS:
        font_path = BUNDLED_FONTS[font_name.lower()]
        if not font_path.exists():
            raise ValueError(
                f"Bundled font '{font_name}' not found at {font_path}. "
                f"Please ensure fonts directory contains the required .ttf files."
            )
        return font_path

    # Otherwise treat as file path
    font_path = Path(font_name)
    if not font_path.exists():
        raise ValueError(
            f"Font file not found: {font_name}\n"
            f"Use bundled fonts: {', '.join(BUNDLED_FONTS.keys())} "
            f"or provide a valid path to a .ttf file."
        )

    return font_path


class RAPIDTextGenerator:
    """Generate ABB RAPID code for drawing text."""

    def __init__(self, font_path: str, text_size_mm: float, point_density_mm: float,
                 workobject: str = "wobj0", style: str = "outline"):
        """
        Initialize the generator.

        Args:
            font_path: Font name (sans/bold/mono) or path to TrueType font file
            text_size_mm: Text height in millimeters
            point_density_mm: Maximum distance between points on curves (mm)
            workobject: WorkObject name for RAPID code
            style: "outline" or "filled"
        """
        # Resolve font path (handles both bundled fonts and custom paths)
        self.font_path = str(resolve_font_path(font_path))
        self.text_size_mm = text_size_mm
        self.point_density_mm = point_density_mm
        self.workobject = workobject
        self.style = style

        # Load font
        try:
            self.font = TTFont(self.font_path)
        except Exception as e:
            raise ValueError(f"Failed to load font '{self.font_path}': {e}")

    def _bezier_to_points(self, p0: Tuple[float, float], p1: Tuple[float, float],
                         p2: Tuple[float, float], p3: Tuple[float, float] = None) -> List[Tuple[float, float]]:
        """
        Convert bezier curve to line segments.

        Args:
            p0: Start point
            p1: Control point 1
            p2: Control point 2 (or end point for quadratic)
            p3: End point (None for quadratic bezier)

        Returns:
            List of points along the curve
        """
        points = []

        # Calculate curve length approximately
        if p3 is None:
            # Quadratic bezier
            approx_length = (
                math.dist(p0, p1) + math.dist(p1, p2)
            )
        else:
            # Cubic bezier
            approx_length = (
                math.dist(p0, p1) + math.dist(p1, p2) + math.dist(p2, p3)
            )

        # Number of segments based on density
        num_segments = max(2, int(approx_length / self.point_density_mm))

        for i in range(num_segments + 1):
            t = i / num_segments

            if p3 is None:
                # Quadratic bezier: B(t) = (1-t)²P0 + 2(1-t)tP1 + t²P2
                x = (1-t)**2 * p0[0] + 2*(1-t)*t * p1[0] + t**2 * p2[0]
                y = (1-t)**2 * p0[1] + 2*(1-t)*t * p1[1] + t**2 * p2[1]
            else:
                # Cubic bezier: B(t) = (1-t)³P0 + 3(1-t)²tP1 + 3(1-t)t²P2 + t³P3
                x = ((1-t)**3 * p0[0] + 3*(1-t)**2*t * p1[0] +
                     3*(1-t)*t**2 * p2[0] + t**3 * p3[0])
                y = ((1-t)**3 * p0[1] + 3*(1-t)**2*t * p1[1] +
                     3*(1-t)*t**2 * p2[1] + t**3 * p3[1])

            points.append((x, y))

        return points

    def _get_text_paths(self, text: str) -> List[List[Tuple[float, float]]]:
        """
        Extract paths from text using fontTools.

        Args:
            text: Text to convert to paths

        Returns:
            List of paths, where each path is a list of (x, y) points
        """
        # Get glyph set
        glyph_set = self.font.getGlyphSet()
        cmap = self.font.getBestCmap()

        # Calculate scale factor (font units to mm)
        # TrueType fonts typically use 1000 or 2048 units per em
        units_per_em = self.font['head'].unitsPerEm
        scale = self.text_size_mm / units_per_em

        all_paths = []
        x_offset = 0

        for char in text:
            # Get glyph name for character
            if ord(char) not in cmap:
                print(f"Warning: Character '{char}' not found in font, skipping")
                continue

            glyph_name = cmap[ord(char)]
            glyph = glyph_set[glyph_name]

            # Use RecordingPen to capture drawing commands
            pen = RecordingPen()
            glyph.draw(pen)

            # Convert drawing commands to paths
            current_path = []
            current_pos = (0, 0)

            for cmd, args in pen.value:
                if cmd == 'moveTo':
                    # Start new path
                    if current_path:
                        all_paths.append(current_path)
                    current_path = []
                    current_pos = args[0]
                    current_path.append((
                        (current_pos[0] + x_offset) * scale,
                        current_pos[1] * scale
                    ))

                elif cmd == 'lineTo':
                    current_pos = args[0]
                    current_path.append((
                        (current_pos[0] + x_offset) * scale,
                        current_pos[1] * scale
                    ))

                elif cmd == 'qCurveTo':
                    # Quadratic bezier curve
                    for i in range(len(args) - 1):
                        p0 = current_pos
                        p1 = args[i]
                        p2 = args[i + 1]

                        # Convert to points
                        curve_points = self._bezier_to_points(
                            ((p0[0] + x_offset) * scale, p0[1] * scale),
                            ((p1[0] + x_offset) * scale, p1[1] * scale),
                            ((p2[0] + x_offset) * scale, p2[1] * scale)
                        )
                        current_path.extend(curve_points[1:])  # Skip first point (duplicate)
                        current_pos = args[i + 1]

                elif cmd == 'curveTo':
                    # Cubic bezier curve
                    p0 = current_pos
                    p1, p2, p3 = args

                    curve_points = self._bezier_to_points(
                        ((p0[0] + x_offset) * scale, p0[1] * scale),
                        ((p1[0] + x_offset) * scale, p1[1] * scale),
                        ((p2[0] + x_offset) * scale, p2[1] * scale),
                        ((p3[0] + x_offset) * scale, p3[1] * scale)
                    )
                    current_path.extend(curve_points[1:])
                    current_pos = p3

                elif cmd == 'closePath':
                    # Close the path by connecting to first point
                    if current_path and current_path[0] != current_path[-1]:
                        current_path.append(current_path[0])

            if current_path:
                all_paths.append(current_path)

            # Advance to next character position
            x_offset += glyph.width

        return all_paths

    def _generate_rapid_code(self, paths: List[List[Tuple[float, float]]],
                            text: str, output_file: str) -> dict:
        """
        Generate RAPID code from paths.

        Args:
            paths: List of paths (each path is a list of points)
            text: Original text (for comments)
            output_file: Output filename

        Returns:
            Dictionary with statistics
        """
        total_points = sum(len(path) for path in paths)
        z_offset = 10.0  # Z offset for pen lift (mm)

        # Estimate time (very rough)
        # Assume 100mm/s average speed, 0.1s per point for fine movement
        estimated_time_sec = total_points * 0.15

        rapid_code = []
        rapid_code.append("MODULE TextDrawing")
        rapid_code.append(f"    ! Generated RAPID code for drawing text: {text}")
        rapid_code.append(f"    ! Font: {Path(self.font_path).name}")
        rapid_code.append(f"    ! Text size: {self.text_size_mm} mm")
        rapid_code.append(f"    ! Point density: {self.point_density_mm} mm")
        rapid_code.append(f"    ! Total points: {total_points}")
        rapid_code.append(f"    ! Estimated time: {estimated_time_sec:.1f} seconds")
        rapid_code.append("")

        # Define robtargets
        rapid_code.append("    ! Define target positions")
        point_index = 0
        for path_idx, path in enumerate(paths):
            rapid_code.append(f"    ! Path {path_idx + 1}/{len(paths)}")
            for pt_idx, (x, y) in enumerate(path):
                rapid_code.append(
                    f"    CONST robtarget p{point_index}:=[[{x:.2f},{y:.2f},0],"
                    f"[0,0,1,0],[-1,0,-1,0],[9E9,9E9,9E9,9E9,9E9,9E9]];"
                )
                point_index += 1

        rapid_code.append("")
        rapid_code.append("    PROC DrawText()")
        rapid_code.append(f"        ! Draw text: {text}")
        rapid_code.append("")

        # Generate movement code
        point_index = 0
        for path_idx, path in enumerate(paths):
            rapid_code.append(f"        ! Path {path_idx + 1}/{len(paths)}")

            if len(path) == 0:
                continue

            # Move to start with pen up
            rapid_code.append(f"        ! Move to start of path (pen up)")
            rapid_code.append(
                f"        MoveL Offs(p{point_index},0,0,{z_offset}),"
                f"v100,z10,tool0\\WObj:={self.workobject};"
            )

            # Lower pen
            rapid_code.append(f"        ! Lower pen")
            rapid_code.append(
                f"        MoveL p{point_index},v50,fine,tool0\\WObj:={self.workobject};"
            )
            point_index += 1

            # Draw the path
            rapid_code.append(f"        ! Draw path")
            for pt_idx in range(1, len(path)):
                # Use fine for precise corners, z1 for smooth curves
                zone = "fine" if pt_idx == len(path) - 1 else "z1"
                rapid_code.append(
                    f"        MoveL p{point_index},v{50 if zone == 'fine' else 100},"
                    f"{zone},tool0\\WObj:={self.workobject};"
                )
                point_index += 1

            # Lift pen
            rapid_code.append(f"        ! Lift pen")
            rapid_code.append(
                f"        MoveL Offs(p{point_index - 1},0,0,{z_offset}),"
                f"v50,z10,tool0\\WObj:={self.workobject};"
            )
            rapid_code.append("")

        rapid_code.append("    ENDPROC")
        rapid_code.append("ENDMODULE")

        # Write to file
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write('\n'.join(rapid_code))

        return {
            'total_points': total_points,
            'total_paths': len(paths),
            'estimated_time_sec': estimated_time_sec,
            'output_file': output_file
        }

    def generate(self, text: str, output_file: str) -> dict:
        """
        Generate RAPID code for the given text.

        Args:
            text: Text to draw
            output_file: Output .mod file path

        Returns:
            Dictionary with statistics
        """
        print(f"Generating RAPID code for text: '{text}'")
        print(f"Font: {self.font_path}")
        print(f"Size: {self.text_size_mm} mm")
        print(f"Style: {self.style}")
        print(f"Point density: {self.point_density_mm} mm")
        print()

        # Extract paths from text
        print("Extracting paths from font...")
        paths = self._get_text_paths(text)

        if not paths:
            raise ValueError("No paths generated from text. Check font and text.")

        print(f"Generated {len(paths)} paths")

        # Generate RAPID code
        print("Generating RAPID code...")
        stats = self._generate_rapid_code(paths, text, output_file)

        return stats


def interactive_mode():
    """Run in interactive mode."""
    print("=== ABB RAPID Text Generator ===")
    print()

    # Get text
    text = input("Enter text to draw: ").strip()
    if not text:
        print("Error: Text cannot be empty")
        return

    # Get font
    print("\nChoose font:")
    print("  1. sans  - DejaVu Sans (clean, general-purpose)")
    print("  2. bold  - DejaVu Sans Bold (thick, for titles)")
    print("  3. mono  - DejaVu Sans Mono (monospace, typewriter style)")
    print("  4. Custom path to .ttf file")
    font_choice = input("Font (1-4, default: 1): ").strip()

    if font_choice == "2":
        font_path = "bold"
    elif font_choice == "3":
        font_path = "mono"
    elif font_choice == "4":
        font_path = input("Enter path to .ttf file: ").strip()
        if not font_path:
            print("Error: Font path cannot be empty")
            return
    else:
        font_path = "sans"

    # Validate font
    try:
        resolve_font_path(font_path)
    except ValueError as e:
        print(f"Error: {e}")
        return

    # Get text size
    try:
        size_input = input("\nText height in mm (default: 100): ").strip()
        text_size = float(size_input) if size_input else 100.0
    except ValueError:
        print("Error: Invalid size")
        return

    # Get style
    print("\nStyle options:")
    print("1. outline - Draw only outlines")
    print("2. filled - Draw filled text (not yet implemented, will use outline)")
    style_input = input("Choose style (1 or 2, default: 1): ").strip()
    style = "filled" if style_input == "2" else "outline"

    # Get point density
    try:
        density_input = input("\nPoint density in mm (smaller = more points, default: 2.0): ").strip()
        density = float(density_input) if density_input else 2.0
    except ValueError:
        print("Error: Invalid density")
        return

    # Get workobject
    workobject = input("\nWorkObject name (default: wobj0): ").strip()
    if not workobject:
        workobject = "wobj0"

    # Get output file
    default_output = f"{text.lower().replace(' ', '_')}.mod"
    output_file = input(f"\nOutput file (default: {default_output}): ").strip()
    if not output_file:
        output_file = default_output

    # Generate
    print("\n" + "="*50)
    try:
        generator = RAPIDTextGenerator(
            font_path=font_path,
            text_size_mm=text_size,
            point_density_mm=density,
            workobject=workobject,
            style=style
        )
        stats = generator.generate(text, output_file)

        print("\n" + "="*50)
        print("SUCCESS!")
        print(f"Output file: {stats['output_file']}")
        print(f"Total points: {stats['total_points']}")
        print(f"Total paths: {stats['total_paths']}")
        print(f"Estimated drawing time: {stats['estimated_time_sec']:.1f} seconds")
        print("="*50)

    except Exception as e:
        print(f"\nError: {e}")
        import traceback
        traceback.print_exc()


def main():
    """Main entry point."""
    parser = argparse.ArgumentParser(
        description="Generate ABB RAPID code for drawing text with a robot",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Bundled Fonts:
  sans - DejaVu Sans (clean, general-purpose)
  bold - DejaVu Sans Bold (thick, for titles)
  mono - DejaVu Sans Mono (monospace, typewriter style)

Examples:
  # Interactive mode
  python rapid_text_generator.py

  # Using bundled fonts
  python rapid_text_generator.py --text "HELLO" --font sans --size 100 --output hello.mod
  python rapid_text_generator.py --text "TITLE" --font bold --size 150 --output title.mod
  python rapid_text_generator.py --text "CODE" --font mono --size 80 --output code.mod

  # Using custom font file
  python rapid_text_generator.py --text "CUSTOM" --font /path/to/font.ttf --size 100 --output custom.mod

  # With all options
  python rapid_text_generator.py --text "ROBOT" --font sans --size 150 --style outline --density 1.5 --workobject wobj0 --output robot.mod
        """
    )

    parser.add_argument('--text', type=str, help='Text to draw')
    parser.add_argument('--font', type=str, help='Font name (sans/bold/mono) or path to .ttf file')
    parser.add_argument('--size', type=float, help='Text height in mm')
    parser.add_argument('--style', type=str, choices=['outline', 'filled'],
                       default='outline', help='Drawing style (default: outline)')
    parser.add_argument('--density', type=float, help='Point density in mm (default: 2.0)')
    parser.add_argument('--workobject', type=str, default='wobj0',
                       help='WorkObject name (default: wobj0)')
    parser.add_argument('--output', type=str, help='Output .mod file')

    args = parser.parse_args()

    # If no arguments provided, run interactive mode
    if len(sys.argv) == 1:
        interactive_mode()
        return

    # Validate required arguments for command-line mode
    if not args.text:
        parser.error("--text is required in command-line mode")
    if not args.font:
        parser.error("--font is required in command-line mode")
    if not args.size:
        parser.error("--size is required in command-line mode")

    if not args.output:
        args.output = f"{args.text.lower().replace(' ', '_')}.mod"

    if not args.density:
        args.density = 2.0

    # Generate
    try:
        generator = RAPIDTextGenerator(
            font_path=args.font,
            text_size_mm=args.size,
            point_density_mm=args.density,
            workobject=args.workobject,
            style=args.style
        )
        stats = generator.generate(args.text, args.output)

        print("\n" + "="*50)
        print("SUCCESS!")
        print(f"Output file: {stats['output_file']}")
        print(f"Total points: {stats['total_points']}")
        print(f"Total paths: {stats['total_paths']}")
        print(f"Estimated drawing time: {stats['estimated_time_sec']:.1f} seconds")
        print("="*50)

    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == '__main__':
    main()
