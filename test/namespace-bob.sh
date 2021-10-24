# create network namespace aegisns
ip netns add AEGISNS
# assing interface enp4s0f0 to aegisns
ip link set enp4s0f0 netns AEGISNS
# assing ip to interface
ip netns exec AEGISNS ip add add dev enp4s0f0 10.0.0.2/24
# bring the link up
ip netns exec AEGISNS ip link set dev enp4s0f0 up
# add the standard route for the interface
ip netns exec AEGISNS route add default gw 10.0.0.1 enp4s0f0
# show wether the project has been successful
ip netns exec AEGISNS ifconfig
ip netns exec AEGISNS route 

