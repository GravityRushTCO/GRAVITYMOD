import base64

html = """<!DOCTYPE html><html><head><meta charset="utf-8"><style>body{background:transparent;margin:0;padding:0;display:flex;align-items:center;justify-content:center;width:100vw;height:100vh;overflow:hidden}.container{animation:bounce 2s ease-in-out infinite}.icon{font-size:50px;filter:drop-shadow(0 0 15px currentColor);animation:rgb 4s linear infinite}@keyframes rgb{0%{color:#f00}20%{color:#ff0}40%{color:#0f0}60%{color:#0ff}80%{color:#f0f}100%{color:#f00}}@keyframes bounce{0%,100%{transform:translateY(0)}50%{transform:translateY(-10px)}}</style></head><body><div class="container"><div class="icon">⚡</div></div></body></html>"""

encoded = base64.b64encode(html.encode('utf-8')).decode('utf-8')
print("data:text/html;base64," + encoded)
