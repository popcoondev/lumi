# JSON LED Pattern Format

This document describes the format of JSON LED patterns used in the Lumi project.

## Overview

LED patterns can be defined in JSON format and loaded from files. This allows for easy creation and modification of patterns without changing the code.

## Pattern Index File

The pattern index file (`data/led_patterns.json`) contains a list of all available patterns and their file paths. The format is as follows:

```json
{
  "patterns": [
    {
      "name": "Pattern Name",
      "file": "/led_patterns/pattern_file.json"
    },
    ...
  ]
}
```

## Pattern File Format

Each pattern is defined in a separate JSON file with the following structure:

```json
{
  "name": "Pattern Name",
  "type": "custom",
  "parameters": {
    "loop": true,
    "stepDelay": 300,
    "colorHSV": {
      "h": 0,
      "s": 0,
      "v": 100
    },
    "effects": {
      "fade": {
        "enabled": false
      },
      "blur": {
        "enabled": false
      }
    }
  },
  "steps": [
    {
      "faces": [0, 1, 2],
      "duration": 300
    },
    ...
  ]
}
```

### Pattern Properties

- `name`: The name of the pattern.
- `type`: The type of pattern (currently only "custom" is supported).
- `parameters`: Global parameters for the pattern.
  - `loop`: Whether the pattern should loop.
  - `stepDelay`: The delay between steps in milliseconds.
  - `colorHSV`: The default color for the pattern in HSV format.
    - `h`: Hue (0-255).
    - `s`: Saturation (0-255).
    - `v`: Value/Brightness (0-255).
  - `effects`: Special effects to apply to the pattern.
    - `fade`: Fade effect.
      - `enabled`: Whether the fade effect is enabled.
      - `mode`: The fade mode ("in", "out", or "both").
      - `duration`: The duration of the fade effect in milliseconds.
    - `blur`: Blur effect.
      - `enabled`: Whether the blur effect is enabled.
      - `intensity`: The intensity of the blur effect.
      - `duration`: The duration of the blur effect in milliseconds.
- `steps`: An array of steps that define the pattern.
  - `faces`: An array of face indices to light up. Can be omitted if using `faceSelection`.
  - `faceSelection`: An object that defines how to select faces.
    - `mode`: The selection mode ("all", "random", "sequential").
    - `range`: The range of faces to select from (for "random" mode).
      - `min`: The minimum face index.
      - `max`: The maximum face index.
    - `count`: The number of faces to select (for "random" mode).
  - `colorHSV`: The color for this step in HSV format. If omitted, the default color from `parameters` is used.
  - `duration`: The duration of this step in milliseconds.

## Random Values

Some properties can be defined as random values within a range:

```json
"h": { "min": 0, "max": 255 }
```

This will generate a random value between `min` and `max` for each face or each time the pattern is run.

## Examples

### Sequential Pattern

```json
{
  "name": "Sequential",
  "type": "custom",
  "parameters": {
    "loop": true,
    "stepDelay": 300,
    "colorHSV": {
      "h": 0,
      "s": 0,
      "v": 100
    },
    "effects": {
      "fade": {
        "enabled": false
      },
      "blur": {
        "enabled": false
      }
    }
  },
  "steps": [
    {
      "faces": [0],
      "duration": 300
    },
    {
      "faces": [1],
      "duration": 300
    },
    {
      "faces": [2],
      "duration": 300
    },
    {
      "faces": [3],
      "duration": 300
    },
    {
      "faces": [4],
      "duration": 300
    },
    {
      "faces": [5],
      "duration": 300
    },
    {
      "faces": [6],
      "duration": 300
    },
    {
      "faces": [7],
      "duration": 300
    }
  ]
}
```

### Random Pattern

```json
{
  "name": "Random",
  "type": "custom",
  "parameters": {
    "loop": true,
    "stepDelay": 500,
    "effects": {
      "fade": {
        "enabled": false
      },
      "blur": {
        "enabled": false
      }
    }
  },
  "steps": [
    {
      "faceSelection": {
        "mode": "all"
      },
      "colorHSV": {
        "h": { "min": 0, "max": 255 },
        "s": { "min": 200, "max": 255 },
        "v": { "min": 200, "max": 255 }
      },
      "duration": 500
    }
  ]
}
```

### Pulse Pattern

```json
{
  "name": "Pulse",
  "type": "custom",
  "parameters": {
    "loop": true,
    "stepDelay": 30,
    "colorHSV": {
      "h": 0,
      "s": 0,
      "v": 100
    },
    "effects": {
      "fade": {
        "enabled": true,
        "mode": "both",
        "duration": 1500
      },
      "blur": {
        "enabled": false
      }
    }
  },
  "steps": [
    {
      "faceSelection": {
        "mode": "all"
      },
      "duration": 3000
    }
  ]
}
