Device High Level Design Requirements
=====================================
Version
-------
  * 0.1: First draft: July 12, 2017

Author
------
  * Paul Barrette

Timeline
--------
  * EAR Sept 30, 2017
  * GA Nov 30, 2017

Terminology
-----------
In this document, the previous HDC model will be referred to as
*HDC2.x*.  The new model will be referred to as *HDC.prime*, since the
final name and version has not been defined at the time of this
writing.

The following table maps HDC2.x to HDC.prime terms:

| HDC 2.x          | HDC.prime |
|------------------|-----------|
| actions          | methods   |
|------------------|-----------|
| telemetry        | properties|
|------------------|-----------|
|custom properties | attributes|
|------------------|-----------|
|capability reg    | thing def |
|------------------|-----------|
| alarms           | alarms    |
|------------------|-----------|
| events           |   NA      |
|------------------|-----------|

Model Change
------------
The cloud provider's cloud model uses a "thing definition" and an application to
represent what a device side application does.  That means that the
thing definition and application definition are pre defined on the
cloud, not on the device via capability registration.  Thing definition
contains configuration for:
  * attributes
  * alarms
  * Properties
  * Methods
  * Tunnels (not supported by HDC.prime)
  * Locations

Before a thing definition can be used, an "application" must be created
in the cloud UI.  This application uses a thing definition and
generates an "app token".

Every app that connects to the cloud requires a "thing key" and an
"app token".  The thing key will be generated automatically as
[UUID]-[ID].  When a connection is made from device to cloud, the
thing key and app token are used for authentication.

Cloud provider's UI tracks "things", not devices like HDC2.x.  From the device
point of view, there will be no agent.  The model is designed to be an
connected application (or thing).  All separate applications on a
device will connect to the cloud independently using the default
connection configuration or each app can override this by specifying a
different connection configuration.

API Change
----------
Most APIs for Cloud provider map to HDC2.x APIs.  New APIs will need
to be created for new functionality as required, e.g. trigger alarm
etc.

Tr50 API
--------
Cloud provider supports tr50 protocol for all communications in and
outside of the cloud.  There is a cAPI that can be used to communicate
with the cloud.  Unfortunately, this cAPI is not fit for production
and was mainly designed as a demo and orphaned.  This is not an issue
because the tr50 API provided 3 things:
  * tr50 encoding
  * mqtt client
  * json parser

WR will use the following based on the tr50 work flow:
  * WR APIs that wrap tr50 encoding
  * Paho/libmosquitto will be used as the mqtt client
  * jsmn json parser

Connection Protocols
--------------------
There are several possibilities for device to cloud protocols:
  * MQTT
  * LWM2M
  * REST

Initially, the WR HDC.prime APIs will use MQTT to connect from device
to the cloud.  There may be REST used in some instances, but the REST
model is not ideal for cloud to device connectivity, e.g.
bidirectional communication is non trivial: it requires either polling
the cloud for actions to execute or a REST server on the device side
which has security implications.  LWM2M is not available at the time
of this writing in the telit cloud, but development is underway.

Plumbing Work
-------------
  * HDC.prime APIs will be backward compatible to HDC2.1+ APIs
  * New connected application model, where every app is independent
  * New Plugin framework
  * Forward port all helper applications (iot-relay, iot-update, file
  IO etc.)

Features Not Supported
----------------------
  * Device exclusive actions
Previously, HDC2.x had a single agent per device.  The new model is a
connected application model, there is no agent.  HDC.prime will
support exclusivity relative to the application only.

Configuration
=============
There will be 2 configuration files:
  * iot-connect.cfg used for connectivity, contains proxy information
  * iot.cfg used for the default actions from the "device manager"
  application.
(See details of these 2 config files below.)

Note on the iot.cfg:  the iot.cfg will continue to be delivered as
iot.cfg.example and the integrator will move it to iot.cfg.  All the
device attributes will be removed from iot.cfg and published
separately.  Any connectivity related config in iot.cfg will be moved
out to iot-connect.cfg.  iot.cfg will be limited to the "device
manager" apps configuration file.

Device ID
---------
A unique ID per device is required.  The following are the
requirements for a device ID:
  * must be written to PATH/device_id, where PATH=runtime dir e.g.
  /var/lib/iot
  * must be a single line, alpha numeric, [-:], max e.g. 128 bytes
  * if alphabetic characters are used, they should be lower case
  * if device_id exists when the apps connect, it will be used
  * if it does not exist, generate a UUID and write it to PATH/device_id

The thing key will be based on the device_id.  The API will use this
ID and append an application ID that was passed in at initialization
time.  There are several advantages to this: Simplicity, the user does
not need to figure out their own unique thing key.  Multiple apps on a
device can be sorted in the UI based on the UUID.

In order to support different types of device_ids, e.g. MAC, IMEI
etc., any string can be written to the device_id file and it will be
used, as is,  instead of a UUID.

Regarding duplicate thing keys or cloning,  two apps in the same "org"
cannot use the same thing key.  The first to register will win
and the thing key will be locked.  Many apps can use the same app
token, as far as it makes sense to do so.  But the thing key is a
unique identifier in the org.

Configuration Tools
-------------------
The "iot-control" tool that was used in HDC2.x will be reused for
HDC.prime.  iot-control will be used to provision the connected app
with connection credentials.  It will also generate the required
iot-connect.cfg.

iot-connect.cfg
---------------
The cloud provider uses mqtt ports 1883 for insecure and 8883 for secure
communications.  HDC.prime will use port 8883 by default.
The iot-connect.cfg file will contain the following:
```
{
	"cloud":{
		"host":"api.devicewise.com",
		"port":8883,
		"token":"abcdefghijklm"
	},
	"validate_cloud_cert": "true",
	"ca_bundle_file":"/etc/ssl/certs/ca-certificates.crt",
	"proxy": {
		"host": [proxy host address],
		"port": [proxy port],
		"type": [proxy type],
		"username": [optional: username],
		"password": [optional: password]
	}
}
```

There will be one default iot-connect.cfg file but any app can
have its own config file stored in $CONFIG_DIR (e.g. /etc/iot).  The
application could then pass in the config on STDIN or call
initialize with $clientID and $connect_file.

The config file name only is required.  The IOT library will look in
the $CONFIG_DIR for the file name.

The application token is pre-generated in the cloud UI when an
application is created.

The token is then specified in the iot-connect.cfg file.
iot-control will be used to generate this file and settings.  There
can be many config files.

iot-proxy.cfg
-------------
HDC2.x used iot-proxy.cfg.  For HDC.prime, iot-proxy.cfg will be merged
into iot-connect.cfg.

iot.cfg
-------
The iot.cfg will be updated to remove the connection information and
the ePO specific parameters.  E.g.:
```
{
	"runtime_dir": "/var/lib/iot",
	"log_level": "ALL",
	"actions_enabled": {
		"shutdown_device": true,
		"reboot_device": true,
		"reset_agent": true,
		"file_transfers": true,
		"software_update": true,
		"restore_factory_images": true,
		"decommission_device": true,
		"dump_log_files": true,
		"remote_login" : true
	},
	"upload_remove_on_success": true,  (for default upload dir only)
	"upload_additional_dirs": [
		""
	]
}
```

The iot.cfg will not be required by default.  An iot.cfg.example file
will be provided as it was in HDC2.x.
Regarding the upload_additional_dirs configuration, this may no longer
be required.  The fileIO APIs will support per file selection.

The python interface will support only the following:
  * actions_enabled
  * runtime_dir
  * upload_remove_on_success

OTA Work Flow For HDC.prime
===========================
This document focuses only the device side view of the OTA process.
In the intial stages of developement there will not be a UI work flow
that creates the OTA package.  However, the OTA package is simple
enough that it should be possible to hand-create the package.  Once
the package is created, it can be deployed with a campaign from the
new HDC.prime cloud.

Changes From HDC2.x
-----------------
  * The ota_manifest.spec file is now obsolete and will no longer be
  read/required.
  * a few changes are required for the update.json file
  * the update.json will continue to be inside the OTA package
  * an OTA update package will now be a single archive (e.g. tar.gz)
  * the OTA package can now be delivered via fileIO

Diagram
-------
```
     +-----------------------+
   +-+     Cloud             |
   | |     File Server       |
   | +-----------------------+
  ++-+     Cloud Services    |
  || +------+----------------+
  ||        |
  ||        |
  ||        | 0) Campaign invokes software_update method
  ||        | (target one or more devices)
  ||        |
  || +-+----+----+
+-++-+  Device   +
| || |  Manager  |
| || +-----+-----+
| ||       |
| ||       | 1) software_update(ota_pkg_file_name) invoked
| |+-------+ 2) file.get(ota_pkg_file_name)
| |        | 3) unarchive ota_pkg_file_name to tmpdir
| |        | 4) exec iot-update(tmpdir)
| |        |
| |   +----+---------+
| |   |  iot-update  |
| |   |              |
| |   +----+---------+
| |        |
| +--------+ 5) connect to cloud a [device_id]-iot-update
| +--------+ 6) log events to cloud
|          | 7) call pre-install, install, post-install, err-install hooks
|          |    Assume all software update install actions are complete at this time.
|          | 8) upload debug log to cloud file store
+----------+ 9) responds to device manager with status
```

HDC.prime Work Flow
-------------------
  * By default, every device that supports the software_update action
  will have a thing definition with the method "software_update" defined.
  * The software_update method takes one parameter:
    * file name of the ota package
  * device manager will handle the software_update action
  * A campaign or custom action will trigger the software_update action
  * The action will retrieve the OTA package over http(s) using the file download API
  * update.json file, as in HDC2.x, contains the details of the OTA
  * operation.  Note: update.json will be packaged along with all the software and
  scripts into an archive, e.g. tar.gz.
  * Once the package has been downloaded, the package will be
  unarchived and a temporary path created
  * the temporary path will be used as a parameter when calling
  the iot-update helper application
  * before launching the iot-update helper app, the service will raise
  an alarm indicating an update is about to begin
  * iot-update will function the same way as it did in HDC2.x at this
  point, with a few changes:
    * the iot-update app will connect to the cloud, using the
    device_id generated initially and appending its own client id.
    * the iot-update app will use events to log directly to the cloud
    * the iot-update app should be able to install all HDC.prime software,
    e.g. libiot and default applications, inluding itself, without losing its cloud
    connection
    * iot-update may be statically compiled (prefered) or take a copy of the
    libiot.so so that it can update itself.  The key requirement is
    that it can update itself and not lose cloud connectivity.

Connection Reliability
======================
All services will use MQTT QOS 1 by default.  This will be a compile
time define. This which will guarantee that the cloud receives a
pub/sub at least once.  This is a change from HDC2.x which only
supported QOS 0 (aka fire and forget).  The default mqtt cliet Paho
supports a local store for publications of data just in case the
application crashes.  After restarting the application, the local
store will be published and deleted.  Initially, mqtt will be
supported, but other connection protocols will be added later, e.g.
REST, LWM2M etc.

Achieving Feature Parity With HDC2.x
=====================================
Once the API has be ported, we will be able to write test apps that
will demonstrate functionality with telit.  We will achieve parity in
phases as the APIs are ported in order of priority.  Backward support
will be limited to the APIs, but the helper tools will continue to be
used.

Helper Applications
-------------------
Helper apps, e.g. iot-update, may choose to connect independently to
the cloud.  If so, the app can then report status even during the
update process when the main application is offline.  The iot-update
could also publish in progress log messages to the cloud.

If so, the helper app will connect using the device_id and append its
own string, e.g.:
    * "[thing_key]-iot-update"

The existing helper applications from HDC2.x will be ported to
HDC.prime:

iot-control
-----------
  * iot-control, update the current provisioning to support telit
  * Device manager:
    * update to meet telit parameter requirements and
    default actions.
    * The name of the app is TBD, but the device manager
    functionality may be confined to a plugin and the app itself
    initializes the plugin/s and provides the interation loop.
    * changes to remove single port support, use proxy instead
    * file IO:
      * parameters to support file IO may change TBD
    * It makes more sense to use an API for file IO so that other apps
    can use file IO independently
    * Current upload/download directories will be supported
    * File paths may be specified (optional).  If not, the default
    upload/download directories will be used.
  * OTA
    * CSP and UI interaction will be added later.  All updates will have to be
    hand packaged.  A software_update action will be registered, but
    it will use telit's fileIO infrastruct.
    * ota-manifest.spec will be deprecated, only the update.json file
    will be used.
  * remote login
    * require CSP with the current websocket relay framework in place
    * device side should work the same way
  * new service to provide health of the device (temp, cpu speed, hd
  usage etc.)
  * CPE string will not be used.  We will publish architecture/OS specific
  attributes
  * libosal will be separated from the old agent so that applications
  can link against it.

Feature for small OSS
---------------------
  * Small OS support with minimal functionality, e.g. for OSS like
  vxworks and smaller.  This will be supported by the basic API and
  OSAL.
  * The details for the small OS support are TBD

Feature for native python support
---------------------------------
Previously in HDC2.x, Python support was dervived automatically via
swig.  This was deemed too "C-like" for python developers at large.
Simplify the process by providing a python class that does not require
binary python modules and can be run on any OS supporting python.  The
intent of the python port is to support quick access to other Linux
distros/architectures not currently supported and for evaluating and
prototyping IOT applications.
  * No binary module, pure python only
  * Module dependencies can be added via "pip"
  * Module functions must be as close as possible to HDC WR API
  * Module must work for python 2.7.x (should be 3 compliant)

Good example of a python module:
  * https://github.com/python-diamond/Diamond

Some Value Add Examples
-----------------------
  * Plugin framework:
  An appliction can be very simple with new functionality added via a
  plugin.
  * Audit service:
  Add a check before and/or after an API call or method to make sure
  the application has permission/clearence to execute.
  * Authentication service:
  A way to independently prove that the device is allowed to connect.
  Authentication tokens may be provided, or the device may be
  periodically challenged to prove its identity. (TBD)  This may be
  triggered by the Audit service above.
  * Service launcher
  * Store N Forward for telemetry

Proxy Support
=============
HDC.prime will support proxy servers the same way as HDC2.x did
previously:

  * connect and register over proxy (MQTT)
  * file IO over proxy (libcurl)
  * remote login over proxy (libwebsockets)

Single port is not currently supported.  With the new cloud model, all
traffic out of a device is via MQTT/TLS on port 8883.  If single port 443 is
required, we will need to work with the cloud provider to open another
MQTT broker port on 443.

Limitations
-----------
Not all MQTT clients support proxy servers:

  * Paho C client does not support proxies
  * Mosquitto client supports socks style proxies
  * libwebsockets supports proxies

API Changes
===========
HDC2.x was previously designed as a gateway for applications.  Going
forward, we will be focusing on being a connected application, where
every app is a self contained unit in full control of its connection
to the cloud.  This will simplify a lot of connectivity and transport
issues we had seen with HDC2.x.

We plan to keep backward API support.  Some API commands may be
deprecated in HDC.prime.  If so, they will be clearly marked
"deprecated" in the API documentation and the implementation for
deprecated APIs will log an error message and return and error.
To summarize:
  * APIs will rewritten to use the tr50 protocol
  * iot enums, structs and statuses will all be reused
  * Any APIs that are deprecated will log and return an error.

CAVEAT:
The new cloud provider only supports telemetry of type "float".  HDC2.x
supported type "string".  If string telemetry is really required,
attributes can be used instead of telemetry to handle strings.


File Transfer
-------------
HDC2.x could do file upload and download, but did not support an API for
file transfer, e.g. where an application could originate a file upload
session. However, the HDC.prime will support a file transfer API.
There will be two modes of operation:

  * compat mode == HDC2.x
  * default mode == HDC.prime

The mode will be auto detected based on the new parameters for file
transfer.  The work flow is as follows:

  * File Download:
    * In HDC2.x today, device manager app registers a default callback for file
    upload/download with parameters.
    * The file download action will have a new optional parameter called
    e.g. "download path" which will indicate where to download the
    file to.
    * HDC2.x did not support a download path option.  So, if there
    is no download path, then compat mode will be selected, and download
    to $runtime_dir/download.
    * If the download action is called with a download path specified,
    this will indicate the new default mode.  If the path is writable, then the
    file will be downloaded to the download path specified.  Otherwise
    and error will be returned.
  * File Upload:
    * The file upload action will have an optional parameter called e.g.
    "file name".
    * If this file name is empty, then compat mode will be
    enabled and any files in the $runtime_dir/upload will be archived
    and then deleted.  If a file name is specified, then the new
    default mode will be enabled.  The API will decide if this is an
    absolute path or file name only.  If the file name is an abs path, then if the
    path is writable, the file will be downloaded to a temporary area
    and then copied (TBD).  If the file name has no path, it will be
    downloaded to the default location $runtime_dir/download.

API List Of Function Names
==========================

Connection
----------
  * iot_initialize
  * iot_connect
  * iot_disconnect
  * iot_terminate

Logging
-------
  * iot_error
  * iot_log
  * iot_log_callback_set
  * iot_log_level_set

Telemetry
---------
Telemetry is called "property" in Cloud provider.
  * iot_telemetry_allocate
  * iot_telemetry_deregister
  * iot_telemetry_register
  * iot_telemetry_free
  * iot_telemetry_publish

Telemetry Deprecated/Not implemented
-------------------------------------
The following APIs are noop:
  * iot_telemetry_publish_raw
  * iot_telemetry_attribute_set_raw
  * any telemetry of type "string", use attributes instead

Telemetry Internal Usage
-------------------------
The following APIs will be used to control how the protocol is used:
  * iot_telemetry_attribute_set

Location
--------
  * iot_location_allocate
  * iot_location_free
  * iot_location_set
  * iot_location_accuracy_set
  * iot_location_altitude_accuracy_set
  * iot_location_altitude_set
  * iot_location_heading_set
  * iot_location_source_set
  * iot_location_speed_set
  * iot_location_tag_set

Actions
-------
Cloud provider calls "actions" -> "methods".

  * iot_action_allocate
  * iot_action_deregister
  * iot_action_free
  * iot_action_parameter_add
  * iot_action_parameter_get
  * iot_action_parameter_get_raw
  * iot_action_parameter_set
  * iot_action_parameter_set_raw
  * iot_action_register_callback
  * iot_action_register_command

Action APIs Deprecated/Not Implemented
--------------------------------------
  * iot_action_attribute_set_raw

Action Internal Usage
---------------------
The following APIs will be used to control how the protocol is used:
  * iot_action_attribute_set

Custom Attributes
-----------------
These are new API calls that were never implemented in HDC2.x.  We had
planned to deliver "custom properties" for HDC2.x, but it was descoped.
Now, with the current cloud provider, properties are telemetry, so we
need to change the name to avoid confusion to use "custom attributes".

```
iot_status_t iot_attribute_publish_string(
	iot_attribute_t *attribute,
	iot_transaction_t *txn,
	iot_millisecond_t max_time_out,
	const char *value );

iot_status_t iot_attribute_free(
	iot_property_t *property,
	iot_millisecond_t max_time_out );

iot_property_t *iot_attribute_allocate(
	iot_t* handle,
	const char *name );
```

File Transfer
-------------
Final signature TBD.

```
iot_status_t iot_file_send(
	iot_t *handle,
	iot_transaction_t *txn,
	iot_millisecond_t max_time_out,
	const char *file_path,
	const char *remote_file_name,
	iot_action_callback_t *completion_callback );

iot_status_t iot_file_receive(
	iot_t *handle,
	iot_transaction_t *txn,
	iot_millisecond_t max_time_out,
	const char *remote_file_name,
	const char *file_path,
	iot_action_callback_t *completion_callback );
```

Alarms
------
```
typedef enum iot_severity {
	SEVERITY_0,
	SEVERITY_1,
	SEVERITY_2,
	SEVERITY_3,
	SEVERITY_4,
	SEVERITY_5
} iot_severity_t;

IOT_SECTION iot_status_t iot_alarm_deregister(
	iot_alarm_t *alarm );

IOT_SECTION iot_alarm_t *iot_alarm_register(
	iot_t *handle,
	const char *name,
	);

iot_status_t iot_alarm_publish(
	iot_alarm_t *alarm,
	iot_severity severity,
	const char *msg
);
```

HDC2.0 APIs Deprecated
========================
The original wra_* APIs were deprecated in HDC 2.1 timeframe and will
not be supported in HDC.prime.  The migration path should be from 2.0 ->
2.1/2 APIs.  Then, the APIs from 2.1/2 are compatable with HDC
HDC.prime.

Remote Login Support
====================
From the device point of view, remote login will require a service of
some kind running.  Since the remote login service will be running
over a secure websocket connection, there is no need to use a secure
protocol.  For example, telnet can be used.  The default cloud UI will
support a javascript class that supports a telnet client.  More
protocols will be added in the future, but initially, telnet will be
used.  For a typical Linux OS, enable a telnet server.  Configuration
of the telnet server is the responsibility of the OS maintainer.  For the
purposes of demonstration, a prebuilt busybox binary can be run:

  * https://busybox.net/downloads/binaries/1.21.1/
  * e.g. busybox-x86_64
  * chmod 755 busybox-x86_64

Now, run this busybox instance.  There are some security concerns for
a telnet server.  The important points to keep in mind:
  * this is on demand service and it is taken down after the session
  ends
  * bind only to the local interface so that it is not externally visable
  * if running as root, execute 'login' so that the user must have
  login credentials on the target.
  * running as non-root is an option, but login must be executed by a
  root user/daemon, at least on Ubuntu.

Example 1: Run As Non Root User
-------------------------------
NonRun as a non root user, and drop to a shell on connect, since only
root can run 'login'.  Use an ephemeral port for non root users:

```sh
./busybox-x86_64 telnetd -F -K -l /bin/bash -b 127.0.0.1:2023 &
```

This demo will run bash as soon as it connects, there is no login
required.

Example 2: Run As Root User With Login
--------------------------------------
This will look like a typical telnet login prompt:

```sh
./busybox-x86_64 telnetd -F -K -l /bin/login -b 127.0.0.1:23 &
```


The thing definition must define a method called:
  * remote-access
The device-manager application will handle this callback and invoke
the iot-relay helper application.

Cloud Provider's Support For MQTT Mutual Authentication
=======================================================
To enable mutual authentication for secure profiles, do the following:
  * enable server certificate validation
  * generate x509 client certificate and key with a self signed root
  CA, e.g. client.pem and key (it is easier to generate without the
  passphrase)
  * when connecting with mosquitto, pass in the paths to the
  client.pem/key
  * Extract the signature algorithm from the client.pem file.  This
  will be used on the server side to validate the client.  Remove
  spaces and new lines from the signature.
  * on the cloud, create a thing definition with an attribute called
  "tlssignature"
  * set the default value for tlssignature to the client signature
  generated above

Now, when the application connects, the server can demand a client
certificate and compare the signature with what is defined in the
thing_def->tlssignature attribute.

Openssl tools are required to generate the client certificate and key.
This cert and key can be predefined in a pre-deployment process and
installed on the device via a zero touch provisioning scheme.  The
names of the cert/key pair must be predefined with paths in the
iot-connect.cfg file.

Example Work Flow
-----------------
Generate certs and keys:
```sh
openssl genrsa -out ca.key 4096
openssl req -new -x509 -days 365 -key ca.key -out ca.crt

openssl genrsa -out client.key 4096
openssl req -new -key client.key -out client.csr

# self-signed
openssl x509 -req -days 365 -in client.csr -CA ca.crt -CAkey ca.key -set_serial 01 -out client.crt

openssl pkcs12 -export -clcerts -in client.crt -inkey client.key -out client.p12
openssl pkcs12 -in client.p12 -out client.pem -clcerts
openssl x509 -in client.pem -text -noout
```
The above command will prompt the user for certificate details.  Once
those are entered, the final line will dump the signature, e.g.:
```
    Signature Algorithm: sha256WithRSAEncryption
         51:0c:12:a5:81:52:f5:a7:a1:f3:b9:24:d1:e2:4f:7f:3b:a9:
         c3:f1:ca:5d:2f:7b:a5:d7:54:6a:54:c1:e1:da:db:56:1b:58:
         17:36:e2:16:46:64:fd:c4:12:35:cd:a3:42:2c:cf:2b:14:ed:
         ca:e8:32:57:37:30:aa:bc:b8:f2:54:76:76:c7:9f:33:2c:db:
         93:bd:c9:3a:5b:22:3c:cb:55:ad:da:b4:e7:63:18:de:0b:03:
         d6:69:58:08:07:9b:2a:89:28:34:16:ba:a3:d1:83:f0:fc:94:
         f9:3e:e9:9f:69:48:4c:74:8f:26:32:e1:01:2b:67:36:1e:29:
         1c:aa:87:17:a9:64:04:2a:06:f3:ef:c3:64:4f:db:8d:cf:c4:
         2a:ad:16:bf:a7:f7:bb:b6:9a:1d:07:f3:40:0b:13:4e:43:6a:
         84:f3:7a:74:a7:70:1b:f7:89:56:ea:43:83:25:11:be:bb:e0:
         56:51:1f:52:64:e5:2f:f2:e3:d8:26:47:76:b2:4d:9f:07:ae:
         ec:b0:92:63:de:f0:f2:8a:b3:8d:7d:65:29:31:09:d2:1d:7d:
         18:28:5c:0c:f3:6b:bd:56:13:0f:1c:f1:ac:27:c1:b8:5d:48:
         01:a6:4e:93:19:5d:f0:ae:f1:13:5d:45:ff:b8:a7:8f:b7:97:
         be:ec:c9:42:15:f3:0f:51:b4:ac:83:24:4c:7c:aa:f9:f2:a7:
         ec:2d:12:9a:80:14:14:28:db:d5:88:fd:09:51:50:46:ab:4f:
         37:a8:1c:9f:f5:06:05:ae:67:b4:4a:7a:11:7f:d6:bb:b9:10:
         4a:1c:d0:2c:71:34:62:51:3a:7f:25:30:37:7c:7a:0c:a9:e5:
         ed:c4:fb:ab:9f:12:ea:a6:2f:a1:96:4b:8e:be:1d:e0:f7:77:
         04:47:e2:a6:ab:2f:f0:cf:d4:2a:a0:0f:f6:36:37:82:45:9a:
         3b:7a:0b:d9:79:39:10:40:4b:c4:34:b0:c5:37:19:6d:d4:6a:
         6e:97:ae:e5:88:3a:76:25:4f:69:05:84:cd:86:ca:cb:84:8a:
         f5:d7:dd:2f:c9:c8:9e:f7:58:59:2d:df:b7:e7:9e:50:af:20:
         03:b9:5e:68:18:04:2c:52:11:37:cf:b3:07:a8:9c:86:4f:aa:
         23:8b:81:56:ab:c9:8b:7a:5c:4e:69:40:a4:4c:2a:cd:df:f2:
         c4:1b:cc:12:7b:e8:a5:ad:9e:60:ae:2c:95:3c:91:3c:a9:a0:
         a4:3e:87:ff:07:b2:e3:15:6b:4f:f2:35:a7:5d:37:74:41:1f:
         9f:f3:56:43:d1:51:c0:4d:17:3f:96:00:54:19:ff:bc:ae:09:
         a4:c1:77:8b:c1:88:ff:0f
```
Extract the hex dump above and remove all white space (spaces and new
lines.  E.g. the following line is suitable to be added to a
thing_def->tlssignature attribute as a default value:
```
51:0c:12:a5:81:52:f5:a7:a1:f3:b9:24:d1:e2:4f:7f:3b:a9:c3:f1:ca:5d:2f:7b:a5:d7:54:6a:54:c1:e1:da:db:56:1b:58:17:36:e2:16:46:64:fd:c4:12:35:cd:a3:42:2c:cf:2b:14:ed:ca:e8:32:57:37:30:aa:bc:b8:f2:54:76:76:c7:9f:33:2c:db:93:bd:c9:3a:5b:22:3c:cb:55:ad:da:b4:e7:63:18:de:0b:03:d6:69:58:08:07:9b:2a:89:28:34:16:ba:a3:d1:83:f0:fc:94:f9:3e:e9:9f:69:48:4c:74:8f:26:32:e1:01:2b:67:36:1e:29:1c:aa:87:17:a9:64:04:2a:06:f3:ef:c3:64:4f:db:8d:cf:c4:2a:ad:16:bf:a7:f7:bb:b6:9a:1d:07:f3:40:0b:13:4e:43:6a:84:f3:7a:74:a7:70:1b:f7:89:56:ea:43:83:25:11:be:bb:e0:56:51:1f:52:64:e5:2f:f2:e3:d8:26:47:76:b2:4d:9f:07:ae:ec:b0:92:63:de:f0:f2:8a:b3:8d:7d:65:29:31:09:d2:1d:7d:18:28:5c:0c:f3:6b:bd:56:13:0f:1c:f1:ac:27:c1:b8:5d:48:01:a6:4e:93:19:5d:f0:ae:f1:13:5d:45:ff:b8:a7:8f:b7:97:be:ec:c9:42:15:f3:0f:51:b4:ac:83:24:4c:7c:aa:f9:f2:a7:ec:2d:12:9a:80:14:14:28:db:d5:88:fd:09:51:50:46:ab:4f:37:a8:1c:9f:f5:06:05:ae:67:b4:4a:7a:11:7f:d6:bb:b9:10:4a:1c:d0:2c:71:34:62:51:3a:7f:25:30:37:7c:7a:0c:a9:e5:ed:c4:fb:ab:9f:12:ea:a6:2f:a1:96:4b:8e:be:1d:e0:f7:77:04:47:e2:a6:ab:2f:f0:cf:d4:2a:a0:0f:f6:36:37:82:45:9a:3b:7a:0b:d9:79:39:10:40:4b:c4:34:b0:c5:37:19:6d:d4:6a:6e:97:ae:e5:88:3a:76:25:4f:69:05:84:cd:86:ca:cb:84:8a:f5:d7:dd:2f:c9:c8:9e:f7:58:59:2d:df:b7:e7:9e:50:af:20:03:b9:5e:68:18:04:2c:52:11:37:cf:b3:07:a8:9c:86:4f:aa:23:8b:81:56:ab:c9:8b:7a:5c:4e:69:40:a4:4c:2a:cd:df:f2:c4:1b:cc:12:7b:e8:a5:ad:9e:60:ae:2c:95:3c:91:3c:a9:a0:a4:3e:87:ff:07:b2:e3:15:6b:4f:f2:35:a7:5d:37:74:41:1f:9f:f3:56:43:d1:51:c0:4d:17:3f:96:00:54:19:ff:bc:ae:09:a4:c1:77:8b:c1:88:ff:0f
```

There are a number of issues to keep in mind with this model, e.g.
every unique client cert requires a unique thing_def with the above
process.  This method doesn't provide much more security than
preshared key.  There is no signer comparison, any signature is
accepted, even self signed certs.

