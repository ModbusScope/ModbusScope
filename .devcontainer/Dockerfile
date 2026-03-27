FROM jgeudens/qt-linux:6.8.3_build_1

# Add the same analysis tools installed at runtime in CI
# (code_quality.yml: apt-get install -y clang-tidy / clazy)
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        clang-format \
        clang-tidy \
        clazy \
    && rm -rf /var/lib/apt/lists/*

# Create a non-root dev user for the devcontainer (selected via remoteUser in
# devcontainer.json). Drop any pre-existing account at uid/gid 1000 first to
# avoid conflicts with base images that ship an 'ubuntu' user at that slot.
RUN existing_user=$(getent passwd 1000 | cut -d: -f1); \
    [ -n "$existing_user" ] && userdel -r "$existing_user" 2>/dev/null || true; \
    existing_group=$(getent group 1000 | cut -d: -f1); \
    [ -n "$existing_group" ] && groupdel "$existing_group" 2>/dev/null || true; \
    groupadd --gid 1000 dev \
    && useradd --uid 1000 --gid 1000 --create-home --shell /bin/bash dev

# Reset the base image ENTRYPOINT (start.sh runs Xvfb + `wait -n` as pid 1,
# which is incompatible with devcontainer lifecycle management).
# QT_QPA_PLATFORM=offscreen is already exported by the base image, so no
# display server is needed.
# Use a dedicated non-root user for devcontainer/CI tasks
RUN groupadd --gid 1000 dev \
    && useradd --uid 1000 --gid 1000 --create-home --shell /bin/bash dev
USER dev

# Reset the base image ENTRYPOINT (start.sh runs Xvfb + `wait -n` as pid 1,
# which is incompatible with devcontainer lifecycle management).
ENTRYPOINT []
CMD ["sleep", "infinity"]
