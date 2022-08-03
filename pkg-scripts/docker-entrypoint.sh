#!/bin/sh
set -x


# In Prod, this may be configured with a GID already matching the container
# allowing the container to be run directly as Jenkins. In Dev, or on unknown
# environments, run the container as root to automatically correct docker
# group in container to match the docker.sock GID mounted from the host.
if [ -f /var/lib/overte/.local -a "$(id -u)" = "0" ]; then
	# realign gid
	THIS_OVERTE_GID=`ls -ngd /var/lib/overte/.local | cut -f3 -d' '`
	CUR_OVERTE_GID=`getent group overte | cut -f3 -d: || true`
	if [ ! -z "$THIS_OVERTE_GID" -a "$THIS_OVERTE_GID" != "$CUR_OVERTE_GID" ]; then
		groupmod -g ${THIS_OVERTE_GID} -o overte
	fi

	# realign pid
	THIS_OVERTE_PID=`ls -nd /var/lib/overte/.local | cut -f3 -d' '`
	CUR_OVERTE_PID=`getent passwd overte | cut -f3 -d: || true`
	if [ ! -z "$THIS_OVERTE_PID" -a "$THIS_OVERTE_PID" != "$CUR_OVERTE_PID" ]; then
		usermod -u ${THIS_OVERTE_PID} -o overte
	fi

	if ! groups overte | grep -q overte; then
		usermod -aG overte overte
	fi
fi

chmod 777 /dev/stdout
chmod 777 /dev/stderr

# continue with CMD
exec "$@"
