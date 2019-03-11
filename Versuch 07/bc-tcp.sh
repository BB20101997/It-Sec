sudo bettercap \
--tcp-proxy \
--tcp-proxy-module tcp-proxy-module.rb \
--tcp-proxy-upstream 192.168.1.1:80
#--tcp-proxy-upstream *:80