token=qh3ivl
curl -X POST \
	-F "token=${token}" \
	-F "msg=test" \
	--proxy 10.42.0.1:8080 \
	https://itsec.sbx320.net/chat
