token=qh3ivl
msg="Moin Moin"
curl -X POST \
	-k \
	-d "token=${token}&msg=${msg}" \
	--proxy 10.42.0.1:8080 \
	https://itsec.sbx320.net/chat
