# Genesi OS — CI/CD Workflows

Genesi OS ships through **two strictly separate** pipelines so that fixing the
live ISO can never break updates for installed users, and vice-versa. Both run
inside a `cachyos/cachyos-v3` container (CachyOS repos + keyring trusted), which
is required because several Genesi packages depend on CachyOS-only packages.

> For the full architecture and roadmap, see [docs/ROADMAP.md](../../docs/ROADMAP.md).

---

## 1. Package / Update pipeline — `publish-packages.yml`

Builds the Genesi packages and publishes them to the in-repo pacman registry
that installed systems pull updates from.

**Triggers**
- Push to `main` or `develop` touching `genesi-arch/packages/**`, this workflow,
  `.gitmodules`, or any package submodule pointer (each PKGBUILD sources `HEAD`
  of its submodule, so a pointer bump must rebuild or the repo ships stale code).
- Manual dispatch.

**What it does**
1. Checks out with submodules (recursive).
2. Collects every `depends`/`makedepends`/`checkdepends` from the PKGBUILDs and
   pre-installs them, so `makepkg` never needs `-s` (avoids fragile in-builder `sudo pacman`).
3. Builds each package as an unprivileged `builder` user.
4. Runs `repo-add` to generate the `genesi.db` / `genesi.files` databases.
5. Commits the repository to `genesi-arch/repo/x86_64` on the same branch.

**Channels** — `main` → **stable**, `develop` → **testing**
(end users switch with the `genesi-channel` command).

**Packages built:** `genesi-settings`, `genesi-kde-settings`, `genesi-ai-mode`,
`genesi-update`, `genesi-channel`, `genesi-calamares`, `genesi-calamares-branding`,
`genesi-welcome`.

---

## 2. ISO pipeline — `iso-pipeline.yml`

Builds a fresh, validated `.iso` — completely independent from the package feed.

**Triggers**
- Push to `main` touching ISO inputs: `genesi-arch/**`, the Calamares config
  submodule, or this workflow. **Docs-only commits are skipped** (`!**/*.md`).
- `v*` tags (cuts a GitHub Release).
- Manual dispatch.

**Job 1 — `validate-install`**
Reproduces the Calamares install: a dependency dry-run **plus a real `pacstrap`**
into a throwaway root. A broken package set fails here, *before* a ~30-minute build.

**Job 2 — `build-iso`** (runs only if Job 1 passed)
1. Frees disk space and installs archiso host deps (grub, syslinux, mtools,
   dosfstools, libisoburn, squashfs-tools).
2. Runs `prepare-and-build.sh` → `buildiso.sh -p desktop` → `mkarchiso` as a
   passwordless-sudo `builder` user (the build scripts refuse to run as root).
3. Produces the `.iso`, writes a SHA-256 checksum, and uploads the
   `genesi-os-iso` artifact (retained 14 days).
4. On `v*` tags, attaches the ISO to an auto-generated GitHub Release.

> The post-build checksum/sign step can exit non-zero in CI (no GPG key); the job
> tolerates that and instead verifies the ISO actually exists.

---

## Cutting a release

```bash
git tag v1.0.0
git push origin v1.0.0
```

The ISO pipeline builds the ISO, creates a GitHub Release, and attaches the
`.iso` plus its `.sha256` checksum.

## Downloading a CI build

1. Open **Actions → Genesi ISO Pipeline**.
2. Pick the latest successful run.
3. Download the `genesi-os-iso` artifact from the **Artifacts** section.

---

## Other workflows

- `pr-validation.yml` — lightweight checks on pull requests.
- `build-iso.yml`, `test-build.yml` — **legacy** (pre-CachyOS, Ubuntu/debootstrap
  era). Superseded by `iso-pipeline.yml`; kept for reference only.

## Secrets

No extra secrets are required — the workflows use the automatic `GITHUB_TOKEN`.
