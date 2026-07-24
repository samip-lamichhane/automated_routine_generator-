import re

def process(text):
    # --- SPECIFIC TEXT COLOR REPLACEMENTS ---
    # 1. Button text for red buttons (was #2B2825, make it #FFFFFF)
    text = re.sub(r'background-color: #C1705A;\s*color: #[0-9a-fA-F]{6};', r'background-color: #B0533E;\n            color: #FFFFFF;', text)
    
    # 2. Button text for accent buttons (was #2B2825, make it #3A342C)
    text = re.sub(r'background-color: #C89B6E;\s*color: #[0-9a-fA-F]{6};', r'background-color: #B8875A;\n            color: #3A342C;', text)
    
    # 3. Button text for auto generate button (was #2B2825, make it #3A342C)
    text = re.sub(r'background-color: #8FA876;\s*color: #[0-9a-fA-F]{6};', r'background-color: #6B8F5A;\n            color: #3A342C;', text)
    
    # 4. Text inside course blocks HTML (was #2B2825 and #5A5449)
    text = text.replace("color: #2B2825; line-height: 1.15;", "color: #3A342C; line-height: 1.15;")
    text = text.replace("color: #5A5449;'>%3</div>", "color: #8B8478;'>%3</div>")
    
    # --- GENERAL HEX REPLACEMENTS ---
    # Base background #2B2825 -> #F2F0EC
    text = text.replace("#2B2825", "#F2F0EC")
    
    # Panels #3A362F -> #E8E4DC
    text = text.replace("#3A362F", "#E8E4DC")
    
    # Borders #5A5449 -> #C9C2B4
    text = text.replace("#5A5449", "#C9C2B4")
    
    # Accents #C89B6E -> #B8875A
    text = text.replace("#C89B6E", "#B8875A")
    
    # Hover accents #A9814F -> #A17542
    text = text.replace("#A9814F", "#A17542")
    
    # Primary Text #EDE6DC -> #3A342C
    text = text.replace("#EDE6DC", "#3A342C")
    
    # Secondary Text #A39C8E -> #8B8478
    text = text.replace("#A39C8E", "#8B8478")
    
    # Success Green #8FA876 -> #6B8F5A
    text = text.replace("#8FA876", "#6B8F5A")
    
    # Error / Destructive #C1705A -> #B0533E
    text = text.replace("#C1705A", "#B0533E")
    
    # --- PASTEL COURSE COLORS REPLACEMENT ---
    pastel_map = {
        '"#b56c80"': '"#DDB1BB"',
        '"#8a9d7e"': '"#BDCDA9"',
        '"#c97a63"': '"#E8B9AA"',
        '"#4c9893"': '"#9DCBCA"',
        '"#508bc9"': '"#A8C6EA"',
        '"#ad748c"': '"#D7ADC1"',
        '"#92a884"': '"#C8D8B6"',
        '"#c79075"': '"#E8C1AB"',
        '"#6798b3"': '"#ADC8D8"',
        '"#b87c97"': '"#E1B1C7"'
    }
    for old_color, new_color in pastel_map.items():
        text = text.replace(old_color, new_color)

    return text

with open('mainwindow.cpp', 'r') as f:
    text = f.read()

text = process(text)

with open('mainwindow.cpp', 'w') as f:
    f.write(text)

print("Light mode replacement done.")
