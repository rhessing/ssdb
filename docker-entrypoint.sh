#!/bin/bash
sleep 15

set -e

# config file location
config_file="/usr/local/ssdb/ssdb.conf"

# Slave of none by default
SLAVEOF=""


# Set env controlled variables
REPLICATION_SETUP=${REPLICATION_SETUP:-mastermaster} # default is mastermaster but can be set to masterslave as well
REPLICATION_SPEED=${REPLICATION_SPEED:--1} # default is -1 but can be set to change the speed
REPLICATION_BINLOG=${REPLICATION_BINLOG:-yes} # default is -1 but can be set to change the speed
PORT=${PORT:-8888}
WORK_DIR=${WORK_DIR:-/var/lib/ssdb}
LOG_LEVEL=${LOG_LEVEL:-info}
LOG_OUTPUT=${LOG_OUTPUT:-log.txt}
LOG_ROTATE_SIZE=${LOG_ROTATE_SIZE:-1000000000}
LEVELDB_CACHE_SIZE=${LEVELDB_CACHE_SIZE:-500}
LEVELDB_WB_SIZE=${LEVELDB_CACHE_SIZE:-64}
LEVELDB_COMPACTION_SPEED=${LEVELDB_COMPACTION_SPEED:-1000}
LEVELDB_COMPRESSION=${LEVELDB_COMPRESSION:-yes}

# we use dns service discovery to find other members when in service mode
# and set/override cluster_members provided by ENV
if [ -n "$SERVICE_NAME" ]; then
	# we check, if we have to enable bootstrapping, if we are the only/first node live
	if [ `getent hosts tasks.$SERVICE_NAME|wc -l` > 1 ] ;then
		# bootstrapping gets enabled by empty gcomm string
		# We need to exlcude ourselves to prevent data loops within the replication
		ME=$(getent hosts $(hostname)|awk '{print $1}')

		# we fetch IPs of service members
		for cluster_member in $(getent hosts tasks.$SERVICE_NAME|awk '{print $1}')
		do
			if [ "$cluster_member" == "$ME" ]; then
				continue
			fi

			if [ "$REPLICATION_SETUP" == "mastermaster" ]; then
				REPLICATION_BINLOG="yes"

				SLAVEOF="${SLAVEOF}\tslaveof:\n\t\tid: svc_${cluster_member//./}\n\t\ttype: mirror\n\t\thost: ${cluster_member}\n\t\tport: $PORT\n"

			elif [ "$REPLICATION_SETUP" == "masterslave" ]; then
				REPLICATION_BINLOG="yes"

				# Ugly but sometimes it takes 2 times before the full internal ID shows up from the reverse DNS request
				NODE_ID=$(getent hosts ${cluster_member} | awk '{print $2}' | cut -d . -f2)
				sleep 1
				NODE_ID=$(getent hosts ${cluster_member} | awk '{print $2}' | cut -d . -f2)
				echo $NODE_ID
				if [ $NODE_ID == 1 ]; then
					SLAVEOF="\tslaveof:\n\t\tid: svc_${NODE_ID}\n\t\ttype: sync\n\t\thost: ${cluster_member}\n\t\tport: $PORT\n"
				fi
			fi
		done
	fi

fi


# Base SSDB config file contents
cat << EOF > $config_file
# ssdb-server config
# MUST indent by TAB!

# relative to path of this file, directory must exists
work_dir = $WORK_DIR
pidfile = /run/ssdb.pid

server:
	ip: 0.0.0.0
	port: $PORT

replication:
	binlog: $REPLICATION_BINLOG
	sync_speed: $REPLICATION_SPEED
EOF

echo -e $SLAVEOF >> $config_file

cat << EOF >> $config_file
logger:
	level: $LOG_LEVEL
	output: $LOG_OUTPUT
	rotate:
		size: $LOG_ROTATE_SIZE

leveldb:
	# in MB
	cache_size: $LEVELDB_CACHE_SIZE
	# in MB
	write_buffer_size: $LEVELDB_WB_SIZE
	# in MB/s
	compaction_speed: $LEVELDB_COMPACTION_SPEED
	# yes|no
	compression: $LEVELDB_COMPRESSION
EOF

echo "SSDB configuration completed, starting ssdb"


/usr/local/ssdb/ssdb-server /usr/local/ssdb/ssdb.conf
