package indexer;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoObject;
import com.epam.indigolucene.common.IndigoHolder;
import org.apache.log4j.Logger;
import org.junit.Test;

import java.util.*;

import static indexer.data.generated.TestSchema.MOL;

/**
 *
 * Created by Artem Malykh on 18.04.16.
 */
public class SimilarityTest extends BaseTest {
    private static final String RARE_MOL = "C1(=O)C[C@@]2(C)[C@]([H])(C(C)=C1O)C[C@]1([H])[C@]34CO[C@@](C(OC)=O)([C@@H](O)[C@H](O)[C@]23[H])[C@]4([H])[C@@H](OC(=O)C)C(=O)O1 |&1:3,5,12,14,17,22,24,26,28,30|";
    private static final Logger logger = Logger.getLogger(SimilarityTest.class);

    private static final String[] TOP50RAREMOLSIMILARITIES = {
            "C[C@@]12CC(=O)C(O)=C(C)[C@H]1C[C@@H]1OC(=O)[C@@H](O)[C@@H]3[C@]4(OC[C@@]13[C@H]2[C@H](O)[C@@H]4O)C(=O)OC |&1:1,9,11,15,17,18,21,22,23,25|",
            "C[C@@]12CC(=O)C(O)=C(C)[C@H]1C[C@@H]1OC(=O)C[C@@H]3[C@]4(OC[C@]31[C@H]2C(O)C4O)C(=O)OCC |&1:1,9,11,16,17,20,21|",
            "C[C@@]12CC(=O)C(O)=C(C)[C@H]1C[C@@H]1OC(=O)[C@@H](OC(=O)CC(C)C)[C@@H]3[C@]4(OC[C@@]13[C@H]2[C@H](O)[C@@H]4O)C(=O)OC |&1:1,9,11,15,23,24,27,28,29,31|",
            "C[C@@]12CC(=O)C(O)=C(C)[C@H]1C[C@@H]1OC(=O)[C@@H](OC(C)=O)[C@@H]3[C@@]4(OC[C@@]13[C@H]2[C@H](O)[C@H]4O)C(=O)OC |&1:1,9,11,15,20,21,24,25,26,28|",
            "C[C@@]12CC(=O)C(O)=C(C)[C@H]1C[C@@H]1OC(=O)[C@@H](OC(=O)/C=C(\\C)/C(C)C)[C@@H]3[C@]4(OC[C@@]13[C@H]2[C@H](O)[C@H]4O)C(=O)OC |&1:1,9,11,15,25,26,29,30,31,33|",
            "C[C@@]12CC(=O)C(O)=C(C)[C@H]1C[C@@H]1OC(=O)[C@@H](OC(=O)C=C(C)C)[C@@H]3[C@]4(OC[C@@]13[C@H]2[C@H](O)[C@H]4O)C(=O)OC |&1:1,9,11,15,23,24,27,28,29,31|",
            "C[C@@]12CC(=O)C(O)=C(C)[C@H]1C[C@@H]1OC(=O)[C@@H](OC(=O)/C=C(\\C)/C(C)C)[C@@H]3[C@]4(OC[C@@]13[C@H]2[C@H](O)[C@@H]4O)C(=O)OC |&1:1,9,11,15,25,26,29,30,31,33|",
            "C[C@@]12[C@H]3[C@@H](O)[C@@H](O)[C@]4(OC[C@@]53[C@H]4[C@@H](OC(=O)/C=C(\\C)/C(C)C)C(=O)O[C@@H]5C[C@H]1[C@H](C)C=C(O)C2=O)C(=O)OC |&1:1,2,3,5,7,10,11,12,25,27,28|",
            "C[C@@]12C=C(O)C(=O)C(C)=C1C[C@@H]1OC(=O)[C@@H](OC(C)=O)[C@@H]3[C@]4(OC[C@@]13[C@H]2[C@H](O)[C@@H]4O)C(=O)OC |&1:1,11,15,20,21,24,25,26,28|",
            "CC1(C)C2=C(O)C(=O)[C@@]3(C)[C@@H]4C(=O)O[C@@](C)(C(=O)OC)C(=O)[C@@]4(C)C(=C)C[C@]3(O)[C@@]2(C)CCC1=O |&1:8,10,14,22,27,29|",
            "CC(C)(OC(C)=O)/C(/C)=C/C(=O)O[C@@H]1[C@H]2[C@@]3(OC[C@]42[C@H]([C@@H](O)[C@H]3O)[C@@]2(C)CC(=O)C(O)=C(C)[C@@H]2C[C@H]4OC1=O)C(=O)OC |&1:13,14,15,18,19,20,22,24,33,35|",
            "CC(C)(O)/C(/C)=C/C(=O)O[C@@H]1[C@H]2[C@@]3(OC[C@]42[C@H]([C@@H](O)[C@H]3O)[C@@]2(C)CC(=O)C(O)=C(C)[C@@H]2C[C@H]4OC1=O)C(=O)OC |&1:10,11,12,15,16,17,19,21,30,32|",
            "C[C@@]12[C@H]3[C@@H](O)C(O)[C@]4(OC[C@@]53[C@H]4[C@@H](OC(=O)CCCCC)C(=O)O[C@@H]5C[C@H]1C(C)=CC(=O)[C@H]2O)C(=O)OC |&1:1,2,3,7,10,11,12,24,26,32|",
            "C[C@@]12[C@H]3[C@@H](O)C(O)[C@]4(OC[C@@]53[C@H]4[C@@H](OC(C)=O)C(=O)O[C@@H]5C[C@H]1C(C)=CC(=O)[C@H]2O)C(=O)OC |&1:1,2,3,7,10,11,12,20,22,28|",
            "C[C@@]12[C@H]3[C@@H](O)C(O)[C@]4(OC[C@@]53[C@H]4[C@@H](OC(=O)CC(C)C)C(=O)O[C@@H]5C[C@H]1C(C)=CC(=O)[C@H]2O)C(=O)OC |&1:1,2,3,7,10,11,12,23,25,31|",
            "C[C@@H]1C[C@]2(C)[C@H]3CC(=O)C(OC)=C(C)[C@]3(C)C[C@H](OC(C)=O)[C@H]2[C@@]21[C@]13C[C@](C)(C[C@@H](OC(C)=O)[C@@]1(C)[C@H](OC(C)=O)[C@H](OC(C)=O)[C@@]2(C)O)C(=O)O3 |&1:1,3,5,14,17,22,23,24,26,29,34,36,41,46|",
            "CC1=CC(=O)[C@@H](O)[C@@]2(C)[C@H]1C[C@H]1OC(=O)[C@H](OC(=O)[C@H](C)CC)[C@@H]3[C@@]41CO[C@]3(C)[C@@H](O)[C@H](O)[C@H]24 |&1:5,7,9,11,15,19,23,24,27,29,31,33|",
            "CC[C@@](C)(OC(C)=O)C(=O)O[C@@H]1[C@@H]2[C@@]34CO[C@]2(C)[C@@H](O)[C@H](O)[C@@H]3[C@]2(C)[C@@H](C[C@H]4OC1=O)C(C)=CC(=O)[C@H]2O |&1:2,11,12,13,16,18,20,22,23,25,27,36|",
            "C[C@@]12CC(=O)C(OC(=O)CC(=O)OC3C(=O)C[C@@]4(C)[C@@H]5[C@H](O)[C@H](O)[C@@]6(OC[C@]75[C@@H]6[C@H](OC(=O)C=C(C)C)C(=O)O[C@H]7C[C@@H]4C=3C)C(=O)OC)=C(C)[C@H]1C[C@@H]1OC(=O)[C@@H](OC(=O)C=C(C)C)[C@@H]3[C@]4(OC[C@@]13[C@H]2[C@H](O)[C@@\"\"H]1,17,19,20,22,24,27,28,29,40,42,51,53,57,65,66,69,70,71,73|",
            "CC1=CC(=O)[C@@H](O)[C@@]2(C)[C@H]1C[C@H]1OC(=O)[C@H](O)[C@]3(O)[C@@]41CO[C@]3(C)[C@@H](O)[C@H](O)[C@H]24 |&1:5,7,9,11,15,17,19,22,24,26,28|",
            "C[C@@]12[C@H]3[C@@H](O)[C@H](O)[C@]4(CO)OC[C@]53[C@@H](C[C@H]1C(C)=CC(=O)[C@H]2O)OC(=O)[C@H](O)[C@]54O |&1:1,2,3,5,7,12,13,15,21,26,28|",
            "C[C@@]12[C@H]3[C@@H](O)C(O)[C@]4(OC[C@@]53[C@H]4[C@@H](OC(=O)/C=C(\\C)/C(C)C)C(=O)O[C@@H]5C[C@H]1C(C)=CC(=O)[C@H]2O)C(=O)OC |&1:1,2,3,7,10,11,12,25,27,33|",
            "CC1[C@@H]2CC(=O)O[C@@H]3C[C@H]4[C@H](C)C[C@@H](O)C(=O)[C@]4(C)[C@@H](C(=O)C=1OC)[C@@]32C |&1:2,7,9,10,13,17,19,25|",
            "CC1[C@@H]2CC(=O)O[C@@H]3C[C@H]4[C@H](C)C[C@H](O)C(=O)[C@]4(C)[C@@H](C(=O)C=1OC)[C@@]32C |&1:2,7,9,10,13,17,19,25|",
            "C[C@@]12CC(=O)C(O)=C(C)[C@H]1C[C@@H]1OC(=O)[C@@H](OC(=O)C3C=CC=CC=3)[C@@H]3[C@]4(OC[C@@]13[C@H]2[C@H](O)[C@@H]4O)C(=O)OC |&1:1,9,11,15,25,26,29,30,31,33|",
            "CC1=CC(=O)[C@@H](O)[C@@]2(C)[C@H]1C[C@H]1OC(=O)C[C@]3(O)[C@@]41CO[C@@]3(C)[C@@H](O)[C@H](O)[C@H]24 |&1:5,7,9,11,16,18,21,23,25,27|",
            "CC1[C@@H]2CC(=O)O[C@@H]3C[C@H]4[C@H](C)C[C@@H](O)[C@H](O)[C@]4(C)[C@@H](C(=O)C=1O)[C@@]32C |&1:2,7,9,10,13,15,17,19,24|",
            "CC1=CC(=O)[C@@H](O)[C@@]2(C)[C@H]1C[C@@H](O)[C@@]13CO[C@]4(C)[C@@H]1C(=O)O[C@H]4[C@H](O)[C@H]23 |&1:5,7,9,11,13,16,18,22,23,25|",
            "CC1[C@@H]2CC(=O)O[C@@H]3C[C@H]4[C@H](C)C[C@@H](O)C(=O)[C@]4(C)[C@@H](C(=O)C=1O)[C@@]32C |&1:2,7,9,10,13,17,19,24|",
            "CC1=CC(=O)[C@@H](O)[C@@]2(C)[C@H]1C[C@H]1OC(=O)[C@H](O)[C@]3(O)[C@@]1(C)[C@@H]2[C@@H](O)[C@H](O)[C@@]3(C)O |&1:5,7,9,11,15,17,19,21,22,24,26|",
            "CC1=CC(=O)[C@@H](O)[C@@]2(C)[C@H]1[C@@H](O)[C@H]1OC(=O)[C@@]3(O)[C@@H](C)[C@@H](O)[C@H](O)[C@H]2[C@]31C |&1:5,7,9,10,12,16,18,20,22,24,25|",
            "CC1=CC(=O)[C@@H](O)[C@@]2(C)[C@H]1C[C@H]1OC(=O)[C@H](O)[C@@]3(O)[C@@H](C)[C@@H](O)[C@H](O)[C@H]2[C@]31C |&1:5,7,9,11,15,17,19,21,23,25,26|",
            "CC1=CC(=O)[C@@H](O)[C@@]2(C)[C@H]1CC(=O)[C@@]13CO[C@]4(C)[C@@H]1C(=O)O[C@H]4[C@H](O)[C@H]23 |&1:5,7,9,13,16,18,22,23,25|",
            "CC1=CC(=O)[C@@H](O)[C@@]2(C)[C@H]1[C@@H](O)[C@H]1OC(=O)C[C@]3(O)[C@@]41CO[C@@]3(C)[C@@H](O)[C@H](O)[C@H]24 |&1:5,7,9,10,12,17,19,22,24,26,28|",
            "CC1=C(OC)C(=O)[C@@H]2[C@]3(C)[C@@H](C[C@H]4OC(=O)CC1(O)[C@]42C)[C@H](C)C[C@@H](O)C3=O |&1:7,8,10,12,19,21,24|",
            "C[C@@]12CC(=O)C(O[C@H]3O[C@@H](CO)[C@H](O)[C@@H](O)[C@@H]3O)=C(C)[C@H]1C[C@@H]1OC(=O)[C@@H](OC(C)=O)[C@@H]3[C@]4(OC[C@@]13[C@H]2[C@H](O)[C@@H]4O)C(=O)OC |&1:1,7,9,12,14,16,20,22,26,31,32,35,36,37,39|",
            "C[C@@]12[C@H]3[C@@H](O)C(O)[C@]4(OC[C@@]53[C@H]4[C@@H](OC(=O)CC(C)C)C(=O)O[C@@H]5C[C@H]1C(C)=C(O[C@@H]1O[C@H](CO)[C@@H](O)[C@H](O)[C@H]1O)C(=O)[C@H]2O)C(=O)OC |&1:1,2,3,7,10,11,12,23,25,30,32,35,37,39,43|",
            "CC1[C@@H]2CC(=O)O[C@@H]3C[C@H]4[C@H](C)C=C(OC)C(=O)[C@]4(C)[C@@H](C(=O)C=1O)[C@@]32C |&1:2,7,9,10,18,20,25|",
            "CC1=CC(=O)[C@@H]2[C@]3(C)[C@@H](C[C@H]4OC(=O)C[C@@H]1[C@@]24C)[C@H](C)C=C(OC)C3=O |&1:5,6,8,10,15,16,18|",
            "C[C@@]1(O)[C@H](OC)[C@@H](O)[C@H]2[C@]3(C)[C@H]1CC(=O)O[C@@H]3C[C@H]1CC=C(OC)C(=O)[C@@]12C |&1:1,3,6,8,9,11,16,18,26|",
            "CC1[C@@H]2CC(=O)O[C@@H]3C[C@H]4CC=C(OC)C(=O)[C@]4(C)[C@@H](C(=O)C=1OC)[C@@]32C |&1:2,7,9,17,19,25|",
            "CC1[C@@H]2CC(=O)O[C@@H]3C[C@H]4[C@H](C)C=C(OC)C(=O)[C@]4(C)[C@@H](C(=O)C=1OC)[C@@]32C |&1:2,7,9,10,18,20,26|",
            "CC1=CC(=O)[C@@H](O)[C@@]2(C)[C@H]1C[C@H]1OC(=O)[C@H](O)[C@@H]3[C@@]1(C)[C@@H]2[C@H](O)[C@@H](O)[C@@]3(C)O |&1:5,7,9,11,15,17,18,20,21,23,25|",
            "CC(C)(OC(C)=O)/C(/C)=C/C(=O)O[C@@H]1[C@H]2[C@@]3(OC[C@]42[C@H]([C@@H](O)C3O)[C@]2(C)[C@@H](C[C@H]4OC1=O)C(C)=CC(=O)[C@H]2O)C(=O)OC |&1:13,14,15,18,19,20,24,26,28,37|",
            "C[C@@H]1C=C(OC)C(=O)[C@@]2(C)[C@H]1C[C@H]1OC(=O)C[C@@H]3[C@@]1(C)[C@@H]2[C@H](O)C[C@@]3(C)C(=O)[C@@H]1CC(=O)OC1 |&1:1,8,10,12,17,18,20,21,24,28|",
            "CC1=C[C@@]23C[C@@H]4OC(=O)C[C@H]5[C@]4(COC)[C@@H]4C(=O)[C@@](O)([C@](O)(C1=O)[C@]24C)[C@@]35C |&1:3,5,10,11,15,18,20,24,26|",
            "C[C@@]12[C@H]3[C@H](O)[C@@H](OC)[C@H](C)[C@@H]4CC(=O)O[C@H](C[C@H]1[C@H](C)C=C(OC)C2=O)[C@@]43C |&1:1,2,3,5,8,10,15,17,18,26|",
            "C[C@@H]1C=C(OC)C(=O)[C@@]2(C)[C@H]1C[C@H]1OC(=O)C[C@@H]3[C@@]1(C)[C@@H]2[C@H](O)C[C@@]3(C)C(=O)[C@]1(C)CC(=O)OC1 |&1:1,8,10,12,17,18,20,21,24,28|",
            "C[C@@]12[C@H]3CC(=O)O[C@@H]1[C@H](O)[C@H]1[C@H](C)C=C(OC)C(=O)[C@]1(C)[C@H]2[C@H](O)[C@@H](OC)[C@@H]3C |&1:1,2,7,8,10,11,19,21,22,24,27|",
            "CC(C)=C1CC2C(C)(OC(C)=O)C(CCC2(C)CC1=O)OC(=O)C(C)(OC(C)=O)C(C)O"};

    @Test
    public void robustSimilarityMeasure() throws Exception {
        final List<String> res = new LinkedList<>();

        Indigo indigo = IndigoHolder.getIndigo();
        IndigoObject rareMol = indigo.loadMolecule(RARE_MOL);

        //TODO: rewrite when serialization/desearilization to beans will be available
        testCollection.find().filter(MOL.unsafeIsSimilarTo(RARE_MOL).addPostFilter(n -> n > 0.9f)).limit(50).processWith(lst ->
                lst.forEach(item -> res.add(IndigoHolder.getIndigo().unserialize((byte[]) item.get(MOL.getName())).canonicalSmiles()))
        );

        //TODO: to figure out why this size is less than limit
        logger.info(res.size());

        float totalSimilarityDelta = 0;

        for (int i = 0; i < res.size(); i++) {
            try {
                totalSimilarityDelta += indigo.similarity(indigo.loadMolecule(TOP50RAREMOLSIMILARITIES[i]), rareMol) - indigo.similarity(indigo.loadMolecule(res.get(i)), rareMol);
            } catch (Exception e) {
                logger.info("index " + i);
                totalSimilarityDelta += 1;
            }
        }

        logger.info("Absolute similarity difference: " + totalSimilarityDelta);
        logger.info("Relative similarity difference: " + totalSimilarityDelta/res.size());


        Set<String> referenceSet = new HashSet<>();
        referenceSet.addAll(Arrays.asList(TOP50RAREMOLSIMILARITIES));

        res.removeAll(referenceSet);
        logger.info(res.size());

        res.stream().forEach(s -> logger.info(indigo.similarity(indigo.loadMolecule(s), rareMol)));
    }



//    @Test
//    public void printTop50Matches(String m) {
//        String fileName = IndexationIntegrationTest.class.getClassLoader().getResource("all_chemul.sd").getFile();
//
//        IndigoObject rareMol = IndigoHolder.getIndigo().loadMolecule(m);
//        Map<String, Float> simMap = new HashMap<>();
//
//        for (IndigoObject mol : IndigoHolder.getIndigo().iterateSDFile(fileName)) {
//            try {
//                simMap.put(mol.canonicalSmiles(), IndigoHolder.getIndigo().similarity(mol, rareMol));
//            } catch (Exception e) {
//                e.printStackTrace();
//            }
//        }
//
//
//        simMap.entrySet().stream().sorted((o1, o2) -> -(int) Math.signum(o1.getValue() - o2.getValue())).limit(50).forEach(e -> logger.info(e.getKey()));
//
//
//    }
}
