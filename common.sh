# @filename     common.sh
# @description  Common bash variables and functions
# @author       rbrenton@gmail.com

# Get directory that this script resides in.
DIR=$(cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd)
#export PATH=$PATH:$EC2_HOME # Why isn't $EC2_HOME parsed from settings.yml properly?

# Settings filename relative to current directory in YAML format.
SETTINGS_YAML=$DIR/settings.yaml

# Parses YAML file into bash friendly variable assignments.
function parse_yaml {
    local prefix=$2
    local s='[[:space:]]*' w='[a-zA-Z0-9_]*' fs=$(echo @|tr @ '\034')
    sed -ne "s|^\($s\):|\1|" \
         -e "s|^\($s\)\($w\)$s:$s[\"']\(.*\)[\"']$s\$|\1$fs\2$fs\3|p" \
         -e "s|^\($s\)\($w\)$s:$s\(.*\)$s\$|\1$fs\2$fs\3|p"  $1 |
    awk -F$fs '{
        indent = length($1)/4;
        vname[indent] = $2;
        for (i in vname) {if (i > indent) {delete vname[i]}}
        if (length($3) > 0) {
            vn=""; for (i=0; i<indent; i++) {vn=(vn)(vname[i])("_")}
            printf("%s%s%s=\"%s\"\n", toupper("'$prefix'"), toupper(vn), toupper($2), $3);
        }
    }'
}


# Run bash commands as a specific user.
# Usage: run_as $username "$command"
function run_as {
    local user=$1
    local cmd=$2

    if [[ "root" = `whoami` ]]; then
        su - "$user" -c "$cmd"
    elif [[ $user = `whoami` ]]; then
        bash -c "$cmd"
    else
        sudo su - "$user" -c "$cmd"
    fi
}


# Exit script if not specified user.
function exit_neq {
    local user=$1
    if [[ $USER != "$user" ]]; then
        echo "Must be run as $user"
        exit 1
    fi
}


# Escapes string to be used as shell argument.
# Usage: foo=$(shell_quote "$foo")
# Example: a b "c" $d becomes: "a b \"c\" \$d"
function shell_quote {
    local arg=$1
    arg=$(printf '%q' "$arg")
    arg=${arg//\\\ / }
    arg="\"$arg\""
    echo $arg
}


# Get ami group defined in YAML settings for aws > group
function get_node_group {
    local group=""

    if [[ -z "$AWS_AMI" ]]; then
        AWS_AMI=`curl $AWS_META_URI/meta-data/ami-id 2> /dev/null`
    fi

    local key="AWS_GROUP_`echo $AWS_AMI|tr [a-z-] [A-Z_]`"
    eval group="\$$key"

    if [[ -z "$group" ]]; then
        echo $AWS_GROUP_DEFAULT
    else
        echo $group
    fi
}

# Get local network ip address.
function get_local_ip {
    echo `curl $AWS_META_URI/meta-data/local-ipv4 2> /dev/null`
}

# Set name tag in AWS
function ec2_set_name_tag {
    local fqdn=$1

    if [[ -z "$AWS_INSTANCE" ]]; then
        AWS_INSTANCE=`curl $AWS_META_URI/meta-data/instance-id 2> /dev/null`
    fi

    TAG_NAME=`ec2-describe-instances $params |egrep "^TAG\sinstance\s$AWS_INSTANCE\sName\s" |cut -f5`

    if [[ -z "$TAG_NAME" ]]; then
        echo "EC2 Name tag was not set."
        cmd=`which ec2-create-tags`
        if [[ ! -z "$cmd" ]]; then
          echo "Setting EC2 Name tag"
          ec2-create-tags -O $AWS_ACCESS_KEY -W $AWS_SECRET_KEY $AWS_INSTANCE --tag "Name=$fqdn"
        fi
    else
        echo "EC2 Name tag already set."
    fi
}



# __main__


# Make settings from settings.yaml available as uppercase variable names, joined by underscores (_). 
# Example: Deploy:\n  Root: /var/www/www.example.org becomes: DEPLOY_ROOT="/var/www/www.example.org"
eval $(parse_yaml $SETTINGS_YAML)

ARGV=("$@")

# Get current user.
USER=`whoami`

# Add missing _ROOT variables for any _DIR.
dir_bases=`set|egrep -o "^[^[:space:]]+_DIR=.*$"|sed 's/_DIR=.*//'`
for base in $dir_bases; do
    eval base_dir="\$${base}_DIR"
    eval base_root="\$${base}_ROOT"
    if [[ -z "$base_dir" ]]; then
        continue;
    elif [[ ! -z "$base_root" ]]; then
        continue;
    else
        if [[ "$base_dir" =~ "/" ]]; then
            base_root=$base_dir
        else
            base_root=$DEPLOY_ROOT/$base_dir
        fi
        eval ${base}_ROOT=$base_root
    fi
done


# Why aren't these being set from yaml properly?
#if [[ -z "$EC2_HOME" ]]; then
  export EC2_HOME=/opt/aws/apitools/ec2
#fi
#if [[ -z "$JAVA_HOME" ]]; then
  export JAVA_HOME=/usr/lib/jvm/jre
#fi
export PATH=$PATH:$EC2_HOME/bin

