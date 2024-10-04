# How to Deploy Indigo-Service Locally

Follow these steps to deploy indigo-service on your local machine.

## Deployment Steps

1. Set up a local environment variable `HOST_IP` on your machine. Use the following commands based on your OS:

   For Windows: `set HOST_IP=localhost`
   For MacOS/Unix: `export HOST_IP=localhost`

2. In the `ui` folder of the frontend part, find the `.env` file and set up the following variables:

   ```
   REACT_APP_API_POSTGRES=/v2
   REACT_APP_API_ELASTIC=/v3
   ```

3. Build the `ui` folder using `npm` by running the following command:

   ```
   npm build
   ```

   **Note**: Don't forget to remove the old build folder if it exists.

4. Open a command prompt or terminal at the root of `indigo-service`, and use Docker to build images and run them with
   the following commands:

   ```
   docker-compose build --no-cache
   docker-compose up -d
   ```

Now you have successfully deployed indigo-service on your local machine!


