import re

def process(text):
    # Simultaneous replacement using a function
    replacements = {
        '#EFEAE0': '#D9BE8F',  # Main background
        '#E5E2DC': '#C2A066',  # Panel/input background
        '#C4BCA8': '#A6864F',  # Borders
        '#A6713E': '#8C5E30',  # Primary accent
        '#8C5E30': '#6E4A24',  # Hover accents
        '#3A342C': '#1A1610',  # Primary text
        '#8B8478': '#5A4E3C',  # Secondary text
        '#6B8F5A': '#5A7A4A',  # Validator PASS
        '#B0533E': '#9C4632',  # Validator FAIL / Delete buttons
    }
    
    # We need to make sure we don't double replace. 
    # But since no old value is exactly a new value of another (Wait! '#A6713E' -> '#8C5E30' and '#8C5E30' -> '#6E4A24'!)
    # To handle this, we can use a regex that matches any of the keys.
    
    pattern = re.compile('|'.join(replacements.keys()), re.IGNORECASE)
    
    def replacer(match):
        return replacements[match.group(0).upper()]
        
    text = pattern.sub(replacer, text)
    
    # Now fix QPushButton text colors to be white (#FFFFFF) instead of the new dark text (#1A1610)
    # The new dark text would be #1A1610 because #3A342C got replaced by it.
    
    # QPushButton text (general)
    text = re.sub(r'(QPushButton\s*\{[^\}]*color:\s*)#1A1610;', r'\g<1>#FFFFFF;', text)
    
    return text

with open('mainwindow.cpp', 'r') as f:
    text = f.read()

text = process(text)

with open('mainwindow.cpp', 'w') as f:
    f.write(text)

print("Theme updated successfully.")
