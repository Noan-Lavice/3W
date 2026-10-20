```bash
#!/usr/bin/env bash

set -euo pipefail

# ==========================================
# Configuration du projet STM32L476RG
# ==========================================

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_NUMBER_FILE="$PROJECT_DIR/.build_number"

cd "$PROJECT_DIR"

# ==========================================
# Gestion du numéro de build
# ==========================================

increment_build_number() {
    local build_number=0

    if [[ -f "$BUILD_NUMBER_FILE" ]]; then
        read -r build_number < "$BUILD_NUMBER_FILE"

        if [[ ! "$build_number" =~ ^[0-9]+$ ]]; then
            echo "Erreur : numéro de build invalide." >&2
            exit 1
        fi
    fi

    build_number=$((build_number + 1))

    printf '%s\n' "$build_number" > "$BUILD_NUMBER_FILE"

    echo "Numéro de build : $build_number"
}

# ==========================================
# Compilation
# ==========================================

build() {
    local mode="${1:-debug}"

    echo "Compilation en mode $mode..."

    increment_build_number

    pio run -e "$mode"

    echo "Compilation terminée avec succès."
}

# ==========================================
# Téléversement via ST-LINK
# ==========================================

upload() {
    local mode="${1:-debug}"

    echo "Préparation du téléversement..."

    build "$mode"

    pio run -e "$mode" -t upload

    echo "Téléversement terminé avec succès."
}

# ==========================================
# Nettoyage
# ==========================================

clean() {
    echo "Nettoyage des fichiers de compilation..."

    pio run -t clean

    echo "Nettoyage terminé."
}

# ==========================================
# Menu d'utilisation
# ==========================================

usage() {
    echo "Utilisation : $0 {build|upload|clean} [debug|release]"
    echo
    echo "Exemples :"
    echo "  $0 build debug"
    echo "  $0 build release"
    echo "  $0 upload debug"
    echo "  $0 upload release"
    echo "  $0 clean"
}

# ==========================================
# Point d'entrée
# ==========================================

if ! command -v pio >/dev/null 2>&1; then
    echo "Erreur : PlatformIO CLI (pio) est introuvable." >&2
    exit 1
fi

ACTION="${1:-}"
MODE="${2:-debug}"

case "$MODE" in
    debug|release) ;;
    *)
        echo "Erreur : mode invalide : $MODE" >&2
        usage
        exit 1
        ;;
esac

case "$ACTION" in
    build)
        build "$MODE"
        ;;
    upload)
        upload "$MODE"
        ;;
    clean)
        clean
        ;;
    *)
        usage
        exit 1
        ;;
esac
```