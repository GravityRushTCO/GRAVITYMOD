import json

with open('dashboard.html', 'r', encoding='utf-8') as f:
    html = f.read()

# We need to replace the `let dynamicMenuConfig = { ... };` block.
# I'll just write the new JSON block.

new_config = """let dynamicMenuConfig = {
            tabs: [
                { id: 'combat', label: 'COMBAT', items: [
                    { id: 120, label: 'Activer Aimbot', type: 'checkbox' },
                    { id: 132, label: 'Tirer a travers les murs', type: 'checkbox' },
                    { id: 184, label: 'Verification de visibilite', type: 'checkbox' },
                    { id: 185, label: 'Lissage de visee', type: 'slider' },
                    { id: 183, label: 'Os cible', type: 'combo' },
                    { id: 205, label: 'Pas de rechargement', type: 'checkbox' },
                    { id: 300, label: 'Suivre Joueur Auto', type: 'checkbox' },
                    { id: 301, label: 'Suivre Voiture Auto', type: 'checkbox' },
                    { id: 302, label: 'Distance de suivi', type: 'slider' },
                    { id: 502, label: 'Wall Hack (Tir)', type: 'checkbox' },
                    { id: 503, label: 'Pas de Recul', type: 'checkbox' },
                    { id: 504, label: 'Super Recul', type: 'checkbox' }
                ]},
                { id: 'visuels', label: 'VISUELS', items: [
                    { id: 121, label: 'Activer ESP', type: 'checkbox' },
                    { id: 194, label: 'Lignes', type: 'checkbox' },
                    { id: 195, label: 'Boites', type: 'checkbox' },
                    { id: 196, label: 'Distance', type: 'checkbox' },
                    { id: 241, label: 'Sante', type: 'checkbox' },
                    { id: 197, label: 'Noms', type: 'checkbox' },
                    { id: 199, label: 'Squelette', type: 'checkbox' },
                    { id: 240, label: 'Viseur (Crosshair)', type: 'checkbox' },
                    { id: 193, label: 'Champ de vision', type: 'slider' }
                ]},
                { id: 'monde', label: 'MONDE', items: [
                    { id: 109, label: 'NoClip / Vol Murs', type: 'checkbox' },
                    { id: 308, label: 'Vol Libre (Fly Mode)', type: 'checkbox' },
                    { id: 13, label: 'God Mode', type: 'checkbox' },
                    { id: 228, label: 'TP Carte / Marqueurs', type: 'checkbox' },
                    { id: 400, label: 'Rapatrier Marqueurs/Jobs', type: 'checkbox' },
                    { id: 410, label: '  -> Rapatrier GPS', type: 'checkbox' },
                    { id: 411, label: '  -> Rapatrier Jobs', type: 'checkbox' },
                    { id: 412, label: '  -> Rapatrier Events', type: 'checkbox' },
                    { id: 413, label: '  -> Rapatrier Quetes/Tutos', type: 'checkbox' },
                    { id: 414, label: '  -> Rapatrier Autres', type: 'checkbox' },
                    { id: 416, label: '  -> [DEV] Auto-Skip Tuto/Quetes', type: 'checkbox' },
                    { id: 110, label: 'NoClip Voiture / Passe-Muraille', type: 'checkbox' },
                    { id: 306, label: 'Coller Voiture sur Cible (Sticky Car)', type: 'checkbox' },
                    { id: 153, label: 'Generer Faux ID Telephone', type: 'checkbox' },
                    { id: 500, label: 'Speed Run (x2)', type: 'checkbox' },
                    { id: 501, label: 'Grand Saut', type: 'checkbox' },
                    { id: 505, label: 'Endurance Infinie', type: 'checkbox' },
                    { id: 506, label: 'S\\'attacher a la voiture', type: 'checkbox' },
                    { id: 507, label: 'Vitesse de Mouvement', type: 'checkbox' },
                    { id: 143, label: 'Vitesse Rapide', type: 'slider' },
                    { id: 510, label: 'Puissance Moteur', type: 'slider' },
                    { id: 511, label: 'Angle Braquage', type: 'slider' },
                    { id: 512, label: 'Freins Max', type: 'checkbox' },
                    { id: 513, label: 'Force Poussee', type: 'slider' },
                    { id: 514, label: 'Vehicule Invincible', type: 'checkbox' },
                    { id: 515, label: 'Essence Infinie', type: 'checkbox' },
                    { id: 516, label: 'Adherence (Grip)', type: 'slider' },
                    { id: 517, label: 'Taille des Roues', type: 'slider' }
                ]},
                { id: 'catalogue', label: 'CATALOGUE', items: [
                    { id: 1001, label: 'Catalogue Vehicules', type: 'button' },
                    { id: 1002, label: 'Catalogue Armes', type: 'button' },
                    { id: 1003, label: 'Catalogue Skins', type: 'button' }
                ]},
                { id: 'teleports', label: 'TELEPORTS', items: [
                    { id: 2001, label: 'Liste de Teleportations', type: 'custom' }
                ]},
                { id: 'couleurs', label: 'COULEURS', items: [
                    { id: 3001, label: 'Parametres Couleurs', type: 'custom' }
                ]}
            ]
        };"""

import re
html = re.sub(r'let dynamicMenuConfig = \{[\s\S]*?\n        \};', new_config, html)

with open('dashboard.html', 'w', encoding='utf-8') as f:
    f.write(html)
