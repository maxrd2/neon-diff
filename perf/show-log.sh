#/bin/bash

cd "$(dirname "$0")"

mv performance.log performance.bak &>/dev/null

process_log(){
	log=''
	while true; do
		read -d ' ' hash
		if [ "$hash" == "" ]; then break; fi
		read -r msg
		echo "$hash $msg"
		
		gh=$(echo $hash | sed -E 's/\x1B\[[0-9;]*[a-zA-Z]//g')
		rm performance.log &>/dev/null
		git checkout "$gh" -- performance.log &>/dev/null
		if [[ -f performance.log ]]; then
			log2=$(sed -E -e 's/^/\t/g' -e 's/([0-9.]+sec)/\x1B[93m\1\x1B[m/g' -e "s/('[^']+')/\x1B[97m\1\x1B[m/g" performance.log)
			if [[ "$log" != "$log2" ]]; then
				log="$log2"
				echo "$log"
			fi
		fi
	done
}

git log --reverse --color=always --oneline --decorate HEAD~15..HEAD -- | process_log | less -RFX

mv performance.bak performance.log || exit 1
