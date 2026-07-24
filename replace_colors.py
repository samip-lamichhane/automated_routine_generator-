import re

def process(content):
    # Text colors on buttons (was #11111b, make it #2B2825 to keep contrast on tan)
    content = content.replace("color: #11111b;", "color: #2B2825;")
    
    # Base background #1e1e2e -> #2B2825
    content = content.replace("#1e1e2e", "#2B2825")
    
    # Pane / active tab #181825 -> #2B2825
    content = content.replace("#181825", "#2B2825")
    
    # Panels / inactive tab / text edits #11111b -> #3A362F
    content = content.replace("#11111b", "#3A362F")
    
    # Grid view headers #1a2035 -> #3A362F
    content = content.replace("#1a2035", "#3A362F")
    
    # #313244 used for backgrounds AND borders
    content = content.replace("background-color: #313244;", "background-color: #3A362F;")
    content = content.replace("background: #313244;", "background: #3A362F;")
    content = content.replace("QColor(\"#313244\")", "QColor(\"#3A362F\")")
    content = content.replace("#313244", "#5A5449")
    
    # Borders #45475a -> #5A5449
    content = content.replace("#45475a", "#5A5449")
    
    # Accents #89b4fa -> #C89B6E
    content = content.replace("#89b4fa", "#C89B6E")
    
    # Hover accents #b4befe, #74c7ec -> #A9814F
    content = content.replace("#b4befe", "#A9814F")
    content = content.replace("#74c7ec", "#A9814F")
    
    # Primary Text #cdd6f4 -> #EDE6DC
    content = content.replace("#cdd6f4", "#EDE6DC")
    
    # Secondary Text #6c7086, #a6adc8 -> #A39C8E
    content = content.replace("#6c7086", "#A39C8E")
    content = content.replace("#a6adc8", "#A39C8E")
    
    # Success Green #a6e3a1, #94e2d5 -> #8FA876
    content = content.replace("#a6e3a1", "#8FA876")
    content = content.replace("#94e2d5", "#8FA876")
    
    # Error / Destructive #f38ba8, #eba0ac -> #C1705A
    content = content.replace("#f38ba8", "#C1705A")
    content = content.replace("#eba0ac", "#C1705A")
    
    return content

filename = "mainwindow.cpp"
with open(filename, 'r') as f:
    text = f.read()

new_text = process(text)

with open(filename, 'w') as f:
    f.write(new_text)

print("Replacement complete.")
