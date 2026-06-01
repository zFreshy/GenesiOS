# Genesi OS - branded fastfetch on interactive bash login shells.
# (fish is the Genesi default; this covers bash users too.)
case $- in
  *i*) ;;
  *) return 2>/dev/null || exit 0 ;;
esac

if [ -z "${GENESI_FASTFETCH_SHOWN:-}" ] && command -v fastfetch >/dev/null 2>&1; then
  export GENESI_FASTFETCH_SHOWN=1
  fastfetch --config /usr/share/genesi/fastfetch/genesi.jsonc
fi
