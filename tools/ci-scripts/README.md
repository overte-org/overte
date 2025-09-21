# Building using Docker containers

To create the Docker image, follow the example command in the relevant Dockerfile.

Then go to the repository root of the Overte repository you want to use for building and run (replacing OS and ARCH):

```bash
docker run -v $(pwd):/overte -it overte/overte-server-build:0.1.7-$OS-$ARCH
```

Enter the `/overte` directory, and run your build following the BUILD_LINUX.md instructions.
