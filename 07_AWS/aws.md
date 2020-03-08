# AWS IoT

We will be using AWS to collect and store data from the Arduino. Temporary AWS accounts will be provided. You can use your own AWS account if you prefer. Some of the services will incur usage charges.

## Login

Log into the AWS Management Console using the link and credentials that were provided for you.

![Screenshot of AWS Login](img/aws-login.png)

Choose the US East (N. Virginia) region using the dropdown on the top right of the screen.

![Screenshot of AWS Region Selection](img/aws-region.png)

## Cloud Formation

We are going to use an AWS Cloud Formation stack to pre-create some infrastructure for us.

From the top left of the screen, click the services menu and type `CloudFormation` into the search box.

![AWS Services - Search for Cloud Formation](img/aws-services-menu-cloudformation.png)

In the CloudFormation console, press the orange `Create Stack` button on the right hand side. 

![Cloud Formation Dashboard](img/aws-cloudformation.png)

Under `Specify Template` choose `Upload a template file` as the template source. Choose `cloud-formation.yml` from the code you downloaded in <<chapter-setup>>. 

![Cloud Formation - Create Stack](img/aws-cloudformation-create-stack.png)

 * Press the `Next button`.
 * Enter `itp` as the stack name and click `next`.
 * Skip the configure stack options, just click `next`. 
 * On the review page,make sure you check the `I acknowledge ...` box at the bottom of the page and click `Create Stack` button. 

![Cloud Formation - Create Stack Complete](img/aws-stack-complete.png)

Once the template finishes running you will have a bunch of new AWS resources that we will use during class.

 * Core IoT Policies
 * DynamoDB Table
 * PostgreSQL Database
 * Lambda Functions
 * IAM Roles and Policies

## Arduino

In order to connect the Arduino to AWS IoT Core, our device needs to authenticate using a private key and client certificate instead of a username and password. Use the CreateCSR.ino sketch to generate a new private key and certificate signing request (CSR) on the device. The CSR is used to generate a certificate from AWS. Finally we'll use the key and certificate to connect and send data to AWS IoT.

### Create Private Key and CSR

Load the [GenerateCSR.ino](https://github.com/don/ITP-DeviceToDatabase/blob/master/02_Arduino_MQTT/arduino/GenerateCSR/GenerateCSR.ino) sketch on your MRK WiFi 1010.

Open the Serial Monitor and generate a CSR. Press the "Send" button or enter key to accept all the default valyes. The device name is pre-populated with the serial number from the ECCX08 chip. Refer to the [week 2 instructions](https://github.com/don/ITP-DeviceToDatabase/blob/master/02_Arduino/exercises/exercise7.md) if you need more details.

![Screenshot of Arduino sketch generating CSR](../02_Arduino/exercises/images/generate-csr.png)

Copy the CSR from the serial monitor and save to a new text file named `csr.txt`. Leave the serial monitor open, since you'll need the device serial number in the next step.

## Create Thing

Before we can connect the Arduino to AWS, we need to tell AWS about the device and generate a certificate. Log into the AWS Management console. Choose `IoT Core` from the Service Menu. In the AWS IoT console, choose `Manage -> Things` from the menu on the left.

https://console.aws.amazon.com/iot/home?region=us-east-1#/thinghub

Press the `Register a Thing` button.
Choose `Create a Single Thing`.

 * Use the Serial Number for your device name. Scroll down and choose Next.
 * Choose `Create with CSR`
 * Choose `csr.txt` in the Finder window
 * Choose `Upload`
 * Attach the `ThingPolicy` to your device
 * Choose `Register Thing`

![Screenshot of successful device registration](img/aws-iot-thing-registered.png)

### Download the Certificate

From the [Manage -> Things](https://console.aws.amazon.com/iot/home?region=us-east-1#/thinghub) screen:
 * Click on the newly created device.
 * Click the Security menu on the left
 * Click the Certificate to show the details
 * Use the Action menu to Download the certificate
 * You will need to paste this certificate into config.h in the next step

![Screenshot of page for downloading the device certificate](img/aws-iot-download-certificate.png)

## Policy

AWS uses IoT policies to give things permission to access AWS IoT resources. Policies can be created using the AWS IoT website, but it's rather cumbersome. The CloudFormation template we ran before create a policy for us.

Open the [AWS IoT Core Policy screen](https://console.aws.amazon.com/iot/home?region=us-east-1#/policyhub) in your web browser. You should see the ThingPolicy that was created by AWS Cloud Formation.

![Screenshot of AWS IoT Policy screen](img/cloudformation-thing-policy.png)

This new policy ensures that when a device connects, the client id of the device matches the Common Name in the certificate. The policy also restricts the topics that a device can use. The device can only publish or subscribe to the topic that begin with `things/${clientId}/`. Clicking on the policy will show you details of the policy document. Your policy should look something like this, but with different arns.

    {
        "Version": "2012-10-17",
        "Statement": [
            {
                "Effect": "Allow",
                "Action": "iot:Connect",
                "Resource": "arn:aws:iot:us-east-1:661516571298:client/${iot:Certificate.Subject.CommonName}"
            },
            {
                "Effect": "Allow",
                "Action": [
                    "iot:Publish", 
                    "iot:Receive"
                ],
                "Resource": "arn:aws:iot:us-east-1:661516571298:topic/things/${iot:ClientId}/*"
            },
            {
                "Effect": "Allow",
                "Action": "iot:Subscribe",
                "Resource": "arn:aws:iot:us-east-1:661516571298:topicfilter/things/${iot:ClientId}/*"
            }
        ]
    }

## AWS.ino

Use the `aws` command line tool to get the AWS IoT MQTT broker URL.

    aws iot describe-endpoint --endpoint-type "iot:Data-ATS"

Using the MQTT broker URL and device certificate you downloaded from AWS, configure [AWS.ino](../02_Arduino_MQTT/arduino/AWS/AWS.ino) sketch and load it onto your Arduino. Make sure you use the certificate and not the CSR. Review the [instructions from week 2](/02_Arduino_MQTT/exercises/exercise7.md#awsino) if necessary.

Open the Arduino Serial Monitor and verify that your device has connected to the wireless network and to AWS.

## Test

Open the AWS IoT console and click on the [Test](https://console.aws.amazon.com/iot/home?region=us-east-1#/test) menu on the left hand side. Subscribe to the wildcard topic `#` and ensure you see data arriving from your device.

![Screenshot of AWS IoT MQTT test page](img/aws-iot-test-subscribe.png)
![Screenshot of AWS IoT MQTT test page with messages from the device](img/aws-iot-test-data.png)

AWS is now successfully receiving data from your device!

Ensure that you can send data back to the Arduino. In the test tab, click `Publish to a topic`. The topic should be `things/${clientId}/led`. The payload should be a value between 0 and 100. Sending different values should adjust the brightness of the LED on your Arduino.

![Screenshot of AWS IoT MQTT test sending data to the device](img/aws-iot-test-send.png)

## Database

Now that we have the device sending and receiving data with AWS using MQTT, we need a database to persist the readings. Click the Services menu at the top of the screen and enter `RDS` intro the search box to navigate to the Amazon Relational Database Service page. Use the [hamburger button](https://en.wikipedia.org/wiki/Hamburger_button) on the top left to expand the menu and choose Databases. 

Click the "Create database" button.

![](img/rds-start.png)

Choose the PostgreSQL engine and select Next

![](img/rds-postgres.png)

Choose Dev/Test and select Next

![](img/rds-dev-test.png)

Check the checkbox in the Free tier info box.

![](img/rds-free.png)

Name the DB instance `itp-d2d`. Enter `postgres` for the master username. Choose a secure password. Click Next.

![](img/rds-settings.png)

Choose `Yes` under Public accessibility

![](img/rds-public.png)

Use you initials for the Database name

![](img/rds-options.png)

Scroll down and choose `Create database`

![](img/rds-create-database.png)

![](img/rds-creating.png)

## Test Connection

Click on the RDS home screen, choose Databases, then click the itp-d2d database instance.

![](img/rds-instance.png)

Scroll down to Connectivity &amp; security to get the Endpoint. This is the hostname of your RDS server.

![](img/rds-endpoint.png)

### Security group rules

The security group rules restrict inbound database connections to your current IP address. To connect from other addresses, you need to modify the `rds-launch-wizard` security group rule. Click on `rds-launch-wizard` and edit the inbound rule to allow PostgreSQL traffic from any host, 0.0.0.0/0. Note that we are making the database public so the AWS Lambda function will be able to connect. A better solutions for a real system is to configure the Lambda function so it can [access resources in the VPC](https://docs.aws.amazon.com/lambda/latest/dg/vpc.html).

![](img/rds-security.png)
![](img/rds-inbound.png)


### psql

Use the endpoint URL and connect to your database using `psql`.

![](img/rds-test-connection.png)

## Schema

Now that you're connected to PostgreSQL, let's create set up the new database and users for our data.

    CREATE DATABASE sensors;
    CREATE USER aws WITH ENCRYPTED PASSWORD 'secret';
    GRANT ALL PRIVILEGES ON DATABASE sensors to aws;

Disconnect from psql using `\q`

Reconnect to the sensors database as user aws

    psql -h itp-d2d.cdgh6zvblo40.us-east-1.rds.amazonaws.com -U aws sensors

Create the environment table to hold data from our sensors

    CREATE TABLE environment (
        id SERIAL,
        device TEXT NOT NULL,
        temperature FLOAT,
        humidity FLOAT,
        recorded_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP);

## Lambda

### Node.js

We will use an AWS Lambda function to insert MQTT data into the database. Create a new Node.js project for the Lambda function.

    mkdir store-sensor-data
    cd store-sensor-data
    npm init -y
    touch index.js

Install the PostgreSQL npm driver.

    npm install pg

Open the `store-sensor-data` directory in a text editor. Replace the contents of index.js with:

    const { Pool } = require('pg');

    const pool = new Pool({
        connectionString: process.env.CONNECTION_STRING
    });

    async function insertRow(event) {
        const query = `INSERT INTO environment 
            (device, temperature, humidity) 
            VALUES ($1,$2,$3)`;  
        const values = [event.device, event.temperature, event.humidity];
        console.log(query, values);

        const result = await pool.query(query, values);
        return result.rowCount;
    };

    exports.handler = async (event) => {    
        await insertRow(event);
        const response = {
            statusCode: 200,
            body: 'OK'
        };
        return response;
    };

Edit `package.json` and add a script to zip the contents of the directory. `"zip": "zip -r lambda.zip * -x *.zip *.log"`. Your package.json file should look like

    {
        "name": "save-sensor-data",
        "version": "1.0.0",
        "description": "",
        "main": "index.js",
        "scripts": {
            "test": "echo \"Error: no test specified\" && exit 1",
            "zip": "zip -r lambda.zip * -x *.zip *.log"
        },
        "author": "",
        "license": "ISC",
        "dependencies": {
            "pg": "^7.9.0"
        }
    }

Save the file and run `npm run zip` to generate a zip file for AWS Lambda.

### Create the Lambda function

Use the Services menu at the top of the AWS console to navigate to the AWS Lambda page. 

![](img/lambda-home.png)

Create a new function named `storeSensorData`. Let AWS create a new role with basic Lambda permissions.

![](img/lambda-create.png)

Change the *code entry type* from *edit code inline* to *Upload a .zip file*. 

![](img/lambda-create-success.png)

Click the upload button and choose lambda.zip. Hit Save on the top right.

![](img/lambda-upload-zip.png)

Scroll down to the Environment Variables section. Create a new variable CONNECTION_STRING with your PostgreSQL connection information. Make sure you adjust the URL for your RDS connection.

    postgres://aws:secret@itp-d2d.cdgh6zvbol40.us-east-1.rds.amazonaws.com/sensors

![](img/lambda-connection-string.png)

Press the save button in the top right.

#### Testing

Use the *Select a test event* combo box and select *Configure test events*. Create an event named `TestSensorData`. The event object should have device, temperature, and humidity. Click create.

![](img/lambda-test-event.png)

Click test.

![](img/lambda-test-success.png)

Assuming the test works, use `psql` to view the data in the database. `SELECT * FROM environment;`

![](img/lambda-select-test-data.png)

## Rules

The Lambda function inserts event data into the database. Next, we need to create a Rule that calls our Lambda function when MQTT data arrives. Use the Services menu and navigate back to IoT Core. Click the Act menu on the left side and create a rule.

![](img/rule-no-rules.png)

Name the rule `environment`

![](img/rule-create.png)

Add the *rule query statement*.

    SELECT topic(2) as device, temperature, humidity FROM 'things/+/environment'
    
The `topic(2)` field gets the 2nd element from the MQTT topic, which in this case, is the device name. See the [AWS IoT SQL Reference](https://docs.aws.amazon.com/iot/latest/developerguide/iot-sql-reference.html) for more details.

![](img/rule-query.png)

Press the add action button. Select "Send a message to a Lambda function" and choose configure action.

![](img/rule-send-to-lambda.png)

Select the "storeSensorData" Lambda function and choose add action.

![](img/rule-store-sensor-data.png)

Choose create rule

![](img/rule-create-success.png)

Now that the rule is deployed, query the environment table again. You should see new data being inserted into the table.

![](img/rule-select-real-data.png)

## Grafana

Log into your local Grafana server. Add a connection to your RDS database. Create a graph to display your sensor data.

![](img/grafana-query.png)
![](img/grafana-graph.png)

Next [Notifications](notifications.md) 

