package com.epam.indigo.predicate;

import com.epam.indigo.BaseElasticTest;
import com.epam.indigo.model.Helpers;
import com.epam.indigo.model.IndigoRecordReaction;
import com.epam.indigo.model.fields.FieldNotFoundException;
import org.junit.jupiter.api.*;
import static org.junit.jupiter.api.Assertions.*;

import java.io.File;
import java.nio.file.Files;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;
import java.util.stream.StreamSupport;


public class ReactionsTest extends BaseElasticTest {

    public static final List<IndigoRecordReaction> reactions = new ArrayList<>();

    @BeforeAll
    public static void setUp() throws Exception {
        setUpRepository(IndigoRecordReaction.class);
        TimeUnit.SECONDS.sleep(5);
        loadReactions();
    }

    protected void check(IndigoRecordReaction target, Iterable<IndigoRecordReaction> found) {
        found.forEach(currentReaction -> {
            assertArrayEquals(currentReaction.getCmf(), target.getCmf());
            assertEquals(
                    currentReaction.getCmf().length,
                    target.getCmf().length
            );
        });
    }


    protected static void loadReactions() throws Exception {
        System.out.println("Loading reactions");
        Files.list(new File("src/test/resources/reactions/rheadb").toPath()).forEach(
                path -> {
                    System.out.println(path.toString());
                    IndigoRecordReaction reaction = Helpers.loadReaction(path.toString());
                    reaction.addCustomObject("File", path.toString());
                    reactions.add(reaction);
                }
        );
        System.out.println("Loaded reactions count:");
        System.out.println(reactions.size());
        repositoryReaction.indexRecords(reactions, reactions.size());
        TimeUnit.SECONDS.sleep(15);
    }

    @AfterAll
    public static void tearDownElastic() {
        elasticsearchContainer.stop();
    }

    @Test
    public void exactMatchTest() {
        IndigoRecordReaction targetReaction = getTargetReaction("38368.rxn");
        Iterable<IndigoRecordReaction> reaction = repositoryReaction.stream()
                .filter(new ExactMatch<>(targetReaction))
                .collect(Collectors.toList());
        check(targetReaction, reaction);
    }

    @Test
    public void similarityMatchTest() {
        IndigoRecordReaction targetReaction = getTargetReaction("38369.rxn");
        Iterable<IndigoRecordReaction> reaction = repositoryReaction.stream()
                .filter(new SimilarityMatch<>(targetReaction, 1))
                .collect(Collectors.toList());
        check(targetReaction, reaction);
    }


    protected IndigoRecordReaction getTargetReaction(String fileName) {
        IndigoRecordReaction targetReaction = null;
        for (IndigoRecordReaction reaction : reactions) {
            try {
                if (reaction.getField("File").toString().endsWith(fileName)) {
                    targetReaction = reaction;
                    break;
                }
            } catch (FieldNotFoundException e) {
                Assertions.fail(e);
            }
        }
        if (targetReaction == null) {
            Assertions.fail(new Exception("Target reaction not found"));
        }
        return targetReaction;
    }

    @Test
    public void euclidMatchTest() {
        IndigoRecordReaction targetReaction = getTargetReaction("42837.rxn");
        Iterable<IndigoRecordReaction> reaction = repositoryReaction.stream()
                .filter(new EuclidSimilarityMatch<>(targetReaction, 1))
                .collect(Collectors.toList());
        System.out.println("Target reaction file:");
        try {
            System.out.println(targetReaction.getField("File").toString());
        } catch (FieldNotFoundException e) {
            System.out.println("- File is not set");
        }
        System.out.println("Result");
        reaction.forEach(r -> {
            try {
                System.out.println(r.getField("File").toString());
            } catch (FieldNotFoundException e) {
                System.out.println("- File is not set");
            }
        });
        System.out.println("Result size:");
        System.out.println(StreamSupport.stream(reaction.spliterator(), false).count());
        check(targetReaction, reaction);
    }

    @Test
    public void tverskyMatchTest() {
        IndigoRecordReaction targetReaction = getTargetReaction("42837.rxn");
        Iterable<IndigoRecordReaction> reaction = repositoryReaction.stream()
                .filter(new TverskySimilarityMatch<>(targetReaction, 1, 1))
                .collect(Collectors.toList());
        check(targetReaction, reaction);
    }
}
