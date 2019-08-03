#!/bin/sh

. /lib/functions.sh
. /usr/share/libubox/jshn.sh

FUNCTION_PATH="/usr/share/sf-mm/functions"
SFMM_PROMPT="sf-mm-scripts>"

sfmm_script_usage() {
cat << EOF
USAGE: $1 command [parameter] [values]
command:
  --json-input
EOF
}

json_get_opt() {
	json_init
	json_load "$1"
	local command target
	json_get_var command "command"
	json_get_var target "class"
	case "$command" in
        set)
            action="set_${target}"
            ;;
        get)
            action="get_${target}"
            ;;
        del)
            action="del_${target}"
            ;;
		end)
			action="end"
			echo "$SFMM_PROMPT"
			;;
		exit)
			exit 0
			;;
	esac
}

case "$1" in
	--json-input)
		action="json_input"
		;;
	*)
		sfmm_script_usage $0
		;;
esac


if [ -z "$action" ]; then
	echo invalid action \'$1\'
	exit 1
fi

sf_mm_scripts=`ls $FUNCTION_PATH`
for script in $sf_mm_scripts; do
	. $FUNCTION_PATH/$script
done

handle_action() {
    local func="$action"

    if [ "$action" != "end" -a "$action" != "json_input" -a "$action" != "" ]; then
        json_select argument
        $func
    fi

	if [ "$action" = "json_input" ]; then
		echo "$SFMM_PROMPT"
		while read CMD; do
			[ -z "$CMD" ] && continue
			json_get_opt "$CMD"
			handle_action
            json_cleanup
		done
		exit 0
	fi
}

handle_action 2>/dev/null
