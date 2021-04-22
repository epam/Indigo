package com.epam.indigo;

import com.sun.jna.Platform;

import java.io.*;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

import static com.epam.indigo.Indigo.LIBINDIGO_DYLIB;

public class IndigoUtils {

    public static String getPathToBinary(Class<?> cls, String dllpath, String path, String filename)
            throws FileNotFoundException {
        if (path == null) {
            String res = extractFromJar(cls, "/" + dllpath, filename);
            if (res != null) return res;
            throw new FileNotFoundException(
                    "Couldn't extract native lib " + filename + " (" + dllpath + ", " + path + ") from jar");
        }
        path = path + File.separator + dllpath + File.separator + filename;
        try {
            return new File(path).getCanonicalPath();
        } catch (IOException e) {
            return path;
        }
    }

    private static String extractFromJar(Class<?> cls, String path, String filename) {
        InputStream stream = cls.getResourceAsStream(path + "/" + filename);

        if (stream == null) return null;

        Path tmpdir_path;
        final File tmpdir;
        final File dllfile;

        try {
            // Clone input stream to calculate its hash and copy to temporary folder
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            byte[] buffer = new byte[4096];
            int len;
            while ((len = stream.read(buffer)) > -1) {
                baos.write(buffer, 0, len);
            }
            baos.flush();
            InputStream is1 = new ByteArrayInputStream(baos.toByteArray());
            InputStream is2 = new ByteArrayInputStream(baos.toByteArray());
            baos.close();

            // Calculate md5 hash string to name temporary folder
            String streamHashString = getHashString(is1);
            is1.close();
            tmpdir_path =
                    Paths.get(System.getProperty("java.io.tmpdir"), "indigo" + streamHashString);
            Files.createDirectories(tmpdir_path);

            // Copy library to temporary folder
            Path dllpath = Paths.get(tmpdir_path.toString(), filename);
            dllfile = dllpath.toFile();
            if (Files.notExists(dllpath)) {
                FileOutputStream outstream = new FileOutputStream(dllfile);
                byte[] buf = new byte[4096];

                while ((len = is2.read(buf)) > 0) outstream.write(buf, 0, len);

                outstream.close();
                is2.close();
            }
        } catch (IOException | NoSuchAlgorithmException e) {
            return null;
        }

        try {
            return dllfile.getCanonicalPath();
        } catch (IOException e) {
            return null;
        }
    }

    private static String getHashString(InputStream input)
            throws NoSuchAlgorithmException, IOException {
        StringBuilder res = new StringBuilder();
        MessageDigest algorithm = MessageDigest.getInstance("MD5");
        algorithm.reset();
        ByteArrayOutputStream buffer = new ByteArrayOutputStream();

        int nRead;
        byte[] data = new byte[4096];

        while ((nRead = input.read(data, 0, data.length)) != -1) {
            buffer.write(data, 0, nRead);
        }
        buffer.flush();

        algorithm.update(buffer.toByteArray());
        byte[] hashArray = algorithm.digest();
        String tmp;
        for (byte b : hashArray) {
            tmp = (Integer.toHexString(0xFF & b));
            if (tmp.length() == 1) {
                res.append("0").append(tmp);
            } else {
                res.append(tmp);
            }
        }
        return res.toString();
    }

    static String getDllPath() {
        String path;
        if (Platform.isWindows()) {
            path = "windows-";
        } else if (Platform.isMac()) {
            path = "darwin-";
        } else if (Platform.isLinux()) {
            path = "linux-";
        } else throw new Error("Operating system not recognized");

        if (Platform.is64Bit()) path += "x86_64";
        else path += "i386";

        return path;
    }

    private static String detectMacMinorVersion(String currentPath) {
        String usingVersion = null;
        String version = System.getProperty("os.version");
        int minorVersion = Integer.parseInt(version.split("\\.")[1]);
        for (int i = minorVersion; i >= 5; i--) {
            if (Indigo.class.getResourceAsStream(
                            File.separator
                                    + currentPath
                                    + "10."
                                    + i
                                    + File.separator
                                    + LIBINDIGO_DYLIB)
                    != null) {
                usingVersion = Integer.toString(i);
                break;
            }
        }
        if (usingVersion == null) {
            throw new Error("Indigo cannot find native libraries for Mac OS X 10." + minorVersion);
        }
        return usingVersion;
    }
}
