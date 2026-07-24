import re

def process(text):
    # Base background #F2F0EC -> #EFEAE0
    text = text.replace("#F2F0EC", "#EFEAE0")
    
    # Panels #E8E4DC -> #E5E2DC
    text = text.replace("#E8E4DC", "#E5E2DC")
    
    # Grid View headers #E5E2DC -> #9A9A9A
    # Since I just replaced #E8E4DC with #E5E2DC, I will specifically target the header styles
    text = re.sub(r'QHeaderView::section:horizontal \{\s*background-color: #E5E2DC;', r'QHeaderView::section:horizontal {\n            background-color: #9A9A9A;', text)
    text = re.sub(r'QHeaderView::section:vertical \{\s*background-color: #E5E2DC;', r'QHeaderView::section:vertical {\n            background-color: #9A9A9A;', text)
    
    # Header text should be dark (#3A342C). Currently it is #B8875A
    text = re.sub(r'(QHeaderView::section:horizontal \{[^}]*color:\s*)#[0-9a-fA-F]{6};', r'\g<1>#3A342C;', text)
    text = re.sub(r'(QHeaderView::section:vertical \{[^}]*color:\s*)#[0-9a-fA-F]{6};', r'\g<1>#3A342C;', text)

    # Borders #C9C2B4 -> #C4BCA8
    text = text.replace("#C9C2B4", "#C4BCA8")
    
    # Accents #B8875A -> #A6713E
    text = text.replace("#B8875A", "#A6713E")
    
    # Hover accents #A17542 -> #8C5E30
    text = text.replace("#A17542", "#8C5E30")
    
    return text

with open('mainwindow.cpp', 'r') as f:
    text = f.read()

text = process(text)

with open('mainwindow.cpp', 'w') as f:
    f.write(text)

print("Light mode v2 replacement done.")
