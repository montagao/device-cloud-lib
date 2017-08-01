#echo "command executed: $0 $@"
#!/bin/bash
#echo "result: $?" >&2
am start -a android.intent.action.VIEW -d https://helixdevicecloud.com
exit 0
