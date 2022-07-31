This directory contains scripts used for building Linux server Docker images.
We are building Debian 11 based images for x86_64 and aarch64.

To build images for both architectures, you first create them with separate tags (e.g. 0.1-aarch64 and 0.1-amd64), push them,
and then you use *docker manifest* to link them together. E.g.:
```bash
docker manifest create overte-org/overte-server-build:0.1 --amend overte-org/overte-server-build:0.1-amd64 --amend overte-org/overte-server-build:0.1-aarch64
```
Then you push the manifest similar to how you would push an image:
```bash
docker manifest push overte-org/overte-server-build:0.1
```

*Dockerfile_build* generates an image that is used to build our Overte Docker Server images. It includes all dependencies for building a server.
Current images are available at https://hub.docker.com/repository/docker/juliangro/overte-server-build

*Dockerfile_runtime_linuxbase* generates a runtime base image for our Server. It includes all dependencies for running a server.
Current images are available at https://hub.docker.com/repository/docker/juliangro/overte-server-base

*Dockerfile_runtime* installs the built server into an image that uses the overte-server-base as a base.
The resulting image can be pushed to a Docker repository, or exported, compressed, and published via other means like the https://public.overte.org/index.html

If you are developing Docker images, you might want to set `DOCKER_BUILDKIT=1` to use a more modern build system that will actually cache each step.
