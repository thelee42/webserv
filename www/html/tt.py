#!/usr/bin/env python3
import os
import sys

# Header HTTP obligatoire
print("Content-Type: text/html\n")

# Page HTML
print("<html>")
print("<head><title>Test CGI</title></head>")
print("<body>")
print("<h1>Script CGI fonctionne ! ✓</h1>")
print("<p>Ce script Python est exécuté par le serveur web.</p>")

# Affichage des variables d'environnement CGI importantes
print("<h2>Variables d'environnement CGI :</h2>")
print("<ul>")

cgi_vars = [
    'REQUEST_METHOD',
    'PATH_INFO',
    'QUERY_STRING',
    'CONTENT_TYPE',
    'CONTENT_LENGTH',
    'SERVER_NAME',
    'SERVER_PORT',
    'SCRIPT_NAME',
    'REMOTE_ADDR',
    'HTTP_USER_AGENT'
]

for var in cgi_vars:
    value = os.environ.get(var, '(non défini)')
    print(f"<li><strong>{var}:</strong> {value}</li>")

print("</ul>")

# Test POST data si présent
if os.environ.get('REQUEST_METHOD') == 'POST':
    content_length = os.environ.get('CONTENT_LENGTH')
    if content_length:
        post_data = sys.stdin.read(int(content_length))
        print("<h2>Données POST reçues :</h2>")
        print(f"<pre>{post_data}</pre>")

print("</body>")
print("</html>")