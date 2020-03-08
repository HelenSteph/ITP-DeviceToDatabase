## Grafana

Use Grafana to visualize your sensor data. Log into your local Grafana server or https://grafana.itpdtd.com. Add a connection to your PostgreSQL RDS database and your Influx database. Create a graph to display your sensor data.

![](img/grafana-query.png)
![](img/grafana-graph.png)

# Glitch & Chart.js

Grafana doesn't natively support DynamoDB. However you can modify the Glitch application from earlier in the semester to visualize data from Dynamo. Open https://glitch.com/edit/#!/itp-arduino-workshop. Use the menu on the top left to "Remix Project" which makes a copy. In your copy of the project open the `.env` file.

Add a username and password for that is used to access the website

	USERNAME="itp"
	PASSWORD="secret"
	
Enter your AWS information.

	AWS_REGION="us-east-1"
	AWS_ACCESS_KEY_ID="AKIAIOSFODNN7EXAMPLE"
	AWS_SECRET_ACCESS_KEY="wJalrXUtnFEMI/K7MDENG/bPxRfiCYEXAMPLEKEY"

Our Dynamo table is different than the original version, so we also need to edit `server.js`. The code expects a column name `deviceId` in the environment table. Our column is named `device`. Edit lines 115 to 121 to fix this.

Original code

	params.KeyConditionExpression = "#deviceId = :device_id";
	params.ExpressionAttributeNames = {
	  "#deviceId": "deviceId"
	};
	params.ExpressionAttributeValues = {
	  ":device_id": id
	};

Suggested Modification

	params.KeyConditionExpression = "#device = :device";
	params.ExpressionAttributeNames = {
	  "#device": "device"
	};
	params.ExpressionAttributeValues = {
	  ":device": id
	};

![](img/glitch-ui.png)