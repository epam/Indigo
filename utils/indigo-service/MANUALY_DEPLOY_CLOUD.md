# How to Deploy Indigo-Service in the Cloud

Follow these steps to deploy indigo-service in a cloud environment.

## Prerequisites

1. On the remote machine, update your installed packages.
2. Install Docker (For example, on AWS
   Linux: [click](https://www.digitalocean.com/community/tutorials/how-to-install-and-use-docker-on-centos-7))

## Deployment Steps

1. Set up a local environment variable `HOST_IP` on the machine where you want to deploy the service. Use the following
   commands based on your OS:

   For Windows: `set HOST_IP=<ip address of instance>`

   For MacOS/Unix: `export HOST_IP=<ip address of instance>`

2. In the `ui` folder of the frontend part, find the `.env` file and set up the following variables:

   ```
   REACT_APP_API_POSTGRES=http://<ip address of instance>:8080/v2
   REACT_APP_API_ELASTIC=http://<ip address of instance>:8080/v3
   ```

3. Build the `ui` folder using `yarn` by running the following command:

   ```
   yarn build
   ```

   **Note**: Don't forget to remove the old build folder if it exists.

4. Zip the `indigo-service` folder.

5. Upload the zipped file to your instance. You can use
   the [scp](https://www.geeksforgeeks.org/scp-command-in-linux-with-examples/) command or apps
   like [FileZilla](https://filezilla-project.org/).

6. Unzip the uploaded file on the remote machine by running:

   ```
   unzip indigo-service.zip
   ```

7. In the root directory of `indigo-service`, use Docker to build images and run them with the following commands:

   ```
   docker-compose build --no-cache
   docker-compose up -d
   ```

8. Check the result in your browser by navigating to:

   ```
   http://<ip-address>:8080
   ```

Now you have successfully deployed indigo-service in the cloud!
