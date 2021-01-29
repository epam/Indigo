package com.epam.indigo.predicate;

import com.epam.indigo.BaseElasticTest;
import com.epam.indigo.model.Helpers;
import com.epam.indigo.model.IndigoRecordReaction;
import org.junit.jupiter.api.*;
import static org.junit.jupiter.api.Assertions.*;

import java.io.File;
import java.nio.file.Files;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;


public class ReactionsTest extends BaseElasticTest {

    public static List<IndigoRecordReaction> reactions = new ArrayList<>();

    @BeforeAll
    public static void setUp() throws Exception {
        setUpRepository(IndigoRecordReaction.class);
        TimeUnit.SECONDS.sleep(5);
        loadReactions();
    }

    protected void check(IndigoRecordReaction target, Iterable<IndigoRecordReaction> found) {
        found.forEach(currentReaction -> {
            assertEquals(
                    currentReaction.getCmf().length,
                    target.getCmf().length
            );
            assertTrue(Arrays.equals(
                    currentReaction.getCmf(),
                    target.getCmf()
            ));
        });
    }


    protected static void loadReactions() throws Exception {
        Files.list(new File("src/test/resources/reactions/rheadb").toPath()).forEach(
                path -> {
                    IndigoRecordReaction reaction = Helpers.loadReaction(path.toString());
                    reactions.add(reaction);
                }
        );
        Iterable<IndigoRecordReaction> reactionIterable = reactions;
        repositoryReaction.indexRecords(reactionIterable, reactions.size());
        TimeUnit.SECONDS.sleep(5);
    }

    @AfterAll
    public static void tearDownElastic() {
        elasticsearchContainer.stop();
    }

    @Test
    public void exactMatchTest() {
        IndigoRecordReaction targetReaction = reactions.get(0);
        Iterable<IndigoRecordReaction> reaction = repositoryReaction.stream()
                .filter(new ExactMatch<IndigoRecordReaction>(targetReaction))
                .collect(Collectors.toList());
        check(targetReaction, reaction);
    }

    @Test
    public void similarityMatchTest() {
        IndigoRecordReaction targetReaction = reactions.get(1);
        Iterable<IndigoRecordReaction> reaction = repositoryReaction.stream()
                .filter(new SimilarityMatch<IndigoRecordReaction>(targetReaction, 1))
                .collect(Collectors.toList());
        check(targetReaction, reaction);
    }

    @Test
    public void euclidMatchTest() {
        IndigoRecordReaction targetReaction = reactions.get(2);
        Iterable<IndigoRecordReaction> reaction = repositoryReaction.stream()
                .filter(new EuclidSimilarityMatch<IndigoRecordReaction>(targetReaction, 1))
                .collect(Collectors.toList());
        check(targetReaction, reaction);
    }

    @Test
    public void tverskyMatchTest() {
        IndigoRecordReaction targetReaction = reactions.get(3);
        Iterable<IndigoRecordReaction> reaction = repositoryReaction.stream()
                .filter(new TverskySimilarityMatch<IndigoRecordReaction>(targetReaction, 1, 1))
                .collect(Collectors.toList());
        check(targetReaction, reaction);
    }
}
