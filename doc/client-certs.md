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

Security Issues
===============
There are a number of issues to keep in mind with this model, e.g.
every unique client cert requires a unique thing_def with the above
process.  This method doesn't provide much more security than
preshared key.  There is no signer comparison, any signature is
accepted, even self signed certs.

