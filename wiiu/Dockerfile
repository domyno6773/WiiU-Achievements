# Reproducible Wii U cross-compiler environment.
# Versioned images match the upstream Wii U ecosystem.
# This image is only used to build on a PC; nothing is installed on the console.
FROM ghcr.io/wiiu-env/devkitppc:20260225

COPY --from=ghcr.io/wiiu-env/wiiupluginsystem:20260418 /artifacts ${DEVKITPRO}

WORKDIR /project
