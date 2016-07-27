package com.epam.indigolucene.commonconfig;

/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

/**
 * Some config constants
 * Created by Artem_Malykh on 8/13/2015.
 */
public class ServiceConfig {
    public static final String SERVICE_URL;
    public static final String STATIC_DIR;
    public static final String GUI_URL;
    public static final String OUTER_IP;
    public static final String UPLOAD_DIR;

    public static Properties getProps() {
        return props;
    }

    private static Properties props = new Properties();

    static {
        InputStream is = ServiceConfig.class.getClassLoader().getResourceAsStream("application.properties");
        if (is != null) {
            try {
                props.load(is);
                SERVICE_URL = props.getProperty("service.url");
                GUI_URL = props.getProperty("gui.url");
                STATIC_DIR = props.getProperty("static.dir");
                UPLOAD_DIR = props.getProperty("upload.dir");
                OUTER_IP = props.getProperty("outer.addr");
            } catch (IOException e) {
                throw new RuntimeException("Can not load application.properties file.");
            }
        } else {
            throw new RuntimeException("Error when finding application.properties.");
        }
    }


}
