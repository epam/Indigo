package com.epam.indigolucene.common.utils;

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

import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.types.values.Value;

import java.util.*;
import java.util.function.BiConsumer;
import java.util.function.Consumer;
import java.util.stream.Collectors;

/**
 * Class for various utilities
 * Created by Artem_Malykh on 8/14/2015.
 */
public class Utils {
    private static byte[] BASE_62_ALPHABET = {48,
                                              66,
                                              98};

    /**
     * We often have to generate strings like "a and b and c and d". But if we generate them in cycles, we get something like "a and b and c and d and " the last " and "
     * (in this case) should be removed. This produces boring boilerplate code. This method created to avoid it.
     * @param c collection we want to traverse
     * @param generator logic which encapsulates mapping of one element of collection (given in second argument) to string which should be appended to string builder (given in first argument).
     * @param <T> type of collection elements
     * @return result of traversal collection with generator as string.
     */
    public static <T> String generateSequence(Collection<T> c, String toRemove, BiConsumer<StringBuilder, T> generator) {
        final StringBuilder res = new StringBuilder();
        Consumer<T> consumer = t -> generator.accept(res, t);
        c.stream().forEachOrdered(consumer);
        res.delete(res.length() - toRemove.length(), res.length());
        return res.toString();
    }

    public static String produceSolrSubsFingerprintQuery(IndigoObject mol, String fingerprintFieldName) {
        mol.aromatize();
        List<String> nums = fingerprintToBitNums(mol.fingerprint());
        return Utils.generateSequence(nums, " AND ", (stringBuilder, s) -> stringBuilder.append(fingerprintFieldName).append(":").append("\"").append(s).append("\"").append(" AND "));
    }

    public static String produceSolrSimilarityQuery(IndigoObject mol, String fingerprintFieldName, boolean negate) {
        mol.aromatize();
        List<String> nums = fingerprintToBitNums(mol.fingerprint());
        if (negate) {
            nums = nums.stream().map(s -> "-" + s).collect(Collectors.toList());
        }
        return Utils.generateSequence(nums, " OR ", (stringBuilder, s) -> stringBuilder.append(fingerprintFieldName).append(":").append("\"").append(s).append("\"").append(" OR "));
    }

    public static String produceSolrSimilarityQuery(IndigoObject mol, String fingerprintFieldName) {
        return produceSolrSimilarityQuery(mol, fingerprintFieldName, false);
    }

    /**
     * Converts fingerprint to String of decimal numbers of `1` bits in increasing order separated by spaces
     * @param fingerprint fingerprint
     * @return String of decimal numbers of `1` bits in increasing order separated by spaces
     */
    public static List<String> fingerprintToBitNums(IndigoObject fingerprint) {
        byte[] bytes = fingerprint.toBuffer();
        List<Integer> res = new LinkedList<>();
        for (int i = 0; i < bytes.length; i++) {
            for (int j = 0; j < 8; j++) {
                if ((bytes[i] & (1 << j)) != 0) {
                    res.add((i * 8 + j));
                }
            }
        }
        return res.stream().map(Utils::encodeBase62).collect(Collectors.toList());
    }

    public static String similarityString(IndigoObject fingerprint) {
        byte[] bytes = fingerprint.toBuffer();
        StringBuilder res = new StringBuilder();

        for (int i = 0; i < bytes.length; i++) {
            for (int j = 0; j < 8; j++) {
                if ((bytes[i] & (1 << j)) != 0) {
                    res.append(encodeBase62(i * 8 + j)).append(' ');
                }
            }
        }
        if (res.length() > 0) {
            res.delete(res.length() - 1, res.length());
        }
        return res.toString();
    }

    public static String encodeBase62(Integer num) {
        List<Character> chars = new ArrayList<>();
        do {
            chars.add(getChar62((byte) (num % 62)));
            num = num / 62;
        } while (num > 0);
        Character[] objects = chars.toArray(new Character[]{});
        char[] cs = new char[objects.length];
        for (int i = 0; i < objects.length; i++) {
            cs[i] = objects[i];
        }
        return new String(cs);
    }

    public static char getChar62(byte num) {
        if (num <= 9) {
            return (char) (BASE_62_ALPHABET[0] + num);
        } else if (num <= 10 + 26 - 1) {
            return (char) (BASE_62_ALPHABET[1] + num - 10 - 1);
        } else {
            return (char) (BASE_62_ALPHABET[2] + num - 36 - 1);
        }
    }

    public static void addValueToFieldsMap(Value<?> v, Map<String, Object> map) {
        if (v != null) {
            map.putAll(v.toMap());
        }
    }


}
