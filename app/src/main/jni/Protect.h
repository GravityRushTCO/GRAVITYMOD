#pragma once

// Lance le thread de surveillance anti-debug / anti-Frida
// À appeler une seule fois depuis JNI_OnLoad
void InitProtection();
