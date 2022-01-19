[CX4H3][#6]	Primary carbon
[CX4H2]([#6])[#6]	Secondary carbon
[CX4H1]([#6])([#6])[#6]	Tertiary carbon
[CX4]([#6])([#6])([#6])[#6]	Quaternary carbon
[CX3;$([H2]),$([H1][#6]),$(C([#6])[#6])]=[CX3;$([H2]),$([H1][#6]),$(C([#6])[#6])]	Alkene
[CX2]#[CX2]	Alkyne
[CX3]=[CX2]=[CX3]	Allene
[ClX1][CX4]	Alkylchloride
[FX1][CX4]	Alkylfluoride
[BrX1][CX4]	Alkylbromide
[IX1][CX4]	Alkyliodide
[OX2H][CX4;!$(C([OX2H])[O,S,#7,#15])]	Alcohol
[OX2H][CX4H2;!$(C([OX2H])[O,S,#7,#15])]	Primary alcohol
[OX2H][CX4H;!$(C([OX2H])[O,S,#7,#15])]	Secondary alcohol
[OX2H][CX4D4;!$(C([OX2H])[O,S,#7,#15])]	Tertiary alcohol
[OX2]([CX4;!$(C([OX2])[O,S,#7,#15,F,Cl,Br,I])])[CX4;!$(C([OX2])[O,S,#7,#15])]	Dialkylether
[SX2]([CX4;!$(C([OX2])[O,S,#7,#15,F,Cl,Br,I])])[CX4;!$(C([OX2])[O,S,#7,#15])]	Dialkylthioether
[OX2](c)[CX4;!$(C([OX2])[O,S,#7,#15,F,Cl,Br,I])]	Alkylarylether
[c][OX2][c]	Diarylether
[SX2](c)[CX4;!$(C([OX2])[O,S,#7,#15,F,Cl,Br,I])]	Alkylarylthioether
[c][SX2][c]	Diarylthioether
[O+;!$([O]~[!#6]);!$([S]*~[#7,#8,#15,#16])]	Oxonium
[NX3+0,NX4+;!$([N]~[!#6]);!$([N]*~[#7,#8,#15,#16])]	Amine
[NX3H2+0,NX4H3+;!$([N][!C]);!$([N]*~[#7,#8,#15,#16])]	Primary aliph amine
[NX3H1+0,NX4H2+;!$([N][!C]);!$([N]*~[#7,#8,#15,#16])]	Secondary aliph amine
[NX3H0+0,NX4H1+;!$([N][!C]);!$([N]*~[#7,#8,#15,#16])]	Tertiary aliph amine
[NX4H0+;!$([N][!C]);!$([N]*~[#7,#8,#15,#16])]	Quaternary aliph ammonium
[NX3H2+0,NX4H3+]c	Primary arom amine
[NX3H1+0,NX4H2+;!$([N][!c]);!$([N]*~[#7,#8,#15,#16])]	Secondary arom amine
[NX3H0+0,NX4H1+;!$([N][!c]);!$([N]*~[#7,#8,#15,#16])]	Tertiary arom amine
[NX4H0+;!$([N][!c]);!$([N]*~[#7,#8,#15,#16])]	Quaternary arom ammonium
[NX3H1+0,NX4H2+;$([N]([c])[C]);!$([N]*~[#7,#8,#15,#16])]	Secondary mixed amine
[NX3H0+0,NX4H1+;$([N]([c])([C])[#6]);!$([N]*~[#7,#8,#15,#16])]	Tertiary mixed amine
[NX4H0+;$([N]([c])([C])[#6][#6]);!$([N]*~[#7,#8,#15,#16])]	Quaternary mixed ammonium
[N+;!$([N]~[!#6]);!$(N=*);!$([N]*~[#7,#8,#15,#16])]	Ammonium
[SX2H][CX4;!$(C([SX2H])~[O,S,#7,#15])]	Alkylthiol
[SX2]([CX4;!$(C([SX2])[O,S,#7,#15,F,Cl,Br,I])])[CX4;!$(C([SX2])[O,S,#7,#15])]	Dialkylthioether
[SX2](c)[CX4;!$(C([SX2])[O,S,#7,#15])]	Alkylarylthioether
[SX2D2][SX2D2]	Disulfide
[OX2H][CX4;!$(C([OX2H])[O,S,#7,#15,F,Cl,Br,I])][CX4;!$(C([N])[O,S,#7,#15])][NX3;!$(NC=[O,S,N])]	1,2-Aminoalcohol
[OX2H][CX4;!$(C([OX2H])[O,S,#7,#15])][CX4;!$(C([OX2H])[O,S,#7,#15])][OX2H]	1,2-Diol
[OX2H][CX4;!$(C([OX2H])([OX2H])[O,S,#7,#15])][OX2H]	1,1-Diol
[OX2H][OX2]	Hydroperoxide
[OX2D2][OX2D2]	Peroxo
[LiX1][#6,#14]	Organolithium compounds
[MgX2][#6,#14]	Organomagnesium compounds
[!#1;!#5;!#6;!#7;!#8;!#9;!#14;!#15;!#16;!#17;!#33;!#34;!#35;!#52;!#53;!#85]~[#6;!-]	Organometallic compounds
[$([CX3H][#6]),$([CX3H2])]=[OX1]	Aldehyde
[#6][CX3](=[OX1])[#6]	Ketone
[$([CX3H][#6]),$([CX3H2])]=[SX1]	Thioaldehyde
[#6][CX3](=[SX1])[#6]	Thioketone
[NX2;$([N][#6]),$([NH]);!$([N][CX3]=[#7,#8,#15,#16])]=[CX3;$([CH2]),$([CH][#6]),$([C]([#6])[#6])]	Imine
[NX3+;!$([N][!#6]);!$([N][CX3]=[#7,#8,#15,#16])]	Immonium
[NX2](=[CX3;$([CH2]),$([CH][#6]),$([C]([#6])[#6])])[OX2H]	Oxime
[NX2](=[CX3;$([CH2]),$([CH][#6]),$([C]([#6])[#6])])[OX2][#6;!$(C=[#7,#8])]	Oximether
[OX2]([#6;!$(C=[O,S,N])])[CX4;!$(C(O)(O)[!#6])][OX2][#6;!$(C=[O,S,N])]	Acetal
[OX2H][CX4;!$(C(O)(O)[!#6])][OX2][#6;!$(C=[O,S,N])]	Hemiacetal
[NX3v3;!$(NC=[#7,#8,#15,#16])]([#6])[CX4;!$(C(N)(N)[!#6])][NX3v3;!$(NC=[#7,#8,#15,#16])][#6]	Aminal
[NX3v3;!$(NC=[#7,#8,#15,#16])]([#6])[CX4;!$(C(N)(N)[!#6])][OX2H]	Hemiaminal
[SX2]([#6;!$(C=[O,S,N])])[CX4;!$(C(S)(S)[!#6])][SX2][#6;!$(C=[O,S,N])]	Thioacetal
[SX2]([#6;!$(C=[O,S,N])])[CX4;!$(C(S)(S)[!#6])][OX2H]	Thiohemiacetal
[NX3v3,SX2,OX2;!$(*C=[#7,#8,#15,#16])][CX4;!$(C([N,S,O])([N,S,O])[!#6])][FX1,ClX1,BrX1,IX1]	Halogen acetal like
[NX3v3,SX2,OX2;!$(*C=[#7,#8,#15,#16])][CX4;!$(C([N,S,O])([N,S,O])[!#6])][FX1,ClX1,BrX1,IX1,NX3v3,SX2,OX2;!$(*C=[#7,#8,#15,#16])]	Acetal like
[NX3v3,SX2,OX2;$(**=[#7,#8,#15,#16])][CX4;!$(C([N,S,O])([N,S,O])[!#6])][FX1,ClX1,BrX1,IX1]	Halogenmethylen ester and similar
[NX3v3,SX2,OX2;$(**=[#7,#8,#15,#16])][CX4;!$(C([N,S,O])([N,S,O])[!#6])][NX3v3,SX2,OX2;!$(*C=[#7,#8,#15,#16])]	NOS methylen ester and similar
[NX3v3,SX2,OX2;$(**=[#7,#8,#15,#16])][CX4;!$(C([N,S,O])([N,S,O])[!#6])][FX1,ClX1,BrX1,IX1,NX3v3,SX2,OX2;!$(*C=[#7,#8,#15,#16])]	Hetero methylen ester and similar
[NX1]#[CX2][CX4;$([CH2]),$([CH]([CX2])[#6]),$(C([CX2])([#6])[#6])][OX2H]	Cyanhydrine
[ClX1][CX3]=[CX3]	Chloroalkene
[FX1][CX3]=[CX3]	Fluoroalkene
[BrX1][CX3]=[CX3]	Bromoalkene
[IX1][CX3]=[CX3]	Iodoalkene
[OX2H][CX3;$([H1]),$(C[#6])]=[CX3]	Enol
[OX2H][CX3;$([H1]),$(C[#6])]=[CX3;$([H1]),$(C[#6])][OX2H]	Endiol
[OX2]([#6;!$(C=[N,O,S])])[CX3;$([H0][#6]),$([H1])]=[CX3]	Enolether
[OX2]([CX3]=[OX1])[#6X3;$([#6][#6]),$([H1])]=[#6X3;!$(C[OX2H])]	Enolester
[NX3;$([NH2][CX3]),$([NH1]([CX3])[#6]),$([N]([CX3])([#6])[#6]);!$([N]*=[#7,#8,#15,#16])][CX3;$([CH]),$([C][#6])]=[CX3]	Enamine
[SX2H][CX3;$([H1]),$(C[#6])]=[CX3]	Thioenol
[SX2]([#6;!$(C=[N,O,S])])[CX3;$(C[#6]),$([CH])]=[CX3]	Thioenolether
[CX3;$([R0][#6]),$([H1R0])](=[OX1])[ClX1]	Acylchloride
[CX3;$([R0][#6]),$([H1R0])](=[OX1])[FX1]	Acylfluoride
[CX3;$([R0][#6]),$([H1R0])](=[OX1])[BrX1]	Acylbromide
[CX3;$([R0][#6]),$([H1R0])](=[OX1])[IX1]	Acyliodide
[CX3;$([R0][#6]),$([H1R0])](=[OX1])[FX1,ClX1,BrX1,IX1]	Acylhalide
[CX3;$([R0][#6]),$([H1R0])](=[OX1])[$([OX2H]),$([OX1-])]	Carboxylic acid
[CX3;$([R0][#6]),$([H1R0])](=[OX1])[OX2][#6;!$(C=[O,N,S])]	Carboxylic ester
[#6][#6X3R](=[OX1])[#8X2][#6;!$(C=[O,N,S])]	Lactone
[CX3;$([H0][#6]),$([H1])](=[OX1])[#8X2][CX3;$([H0][#6]),$([H1])](=[OX1])	Carboxylic anhydride
[$([#6X3H0][#6]),$([#6X3H])](=[!#6])[!#6]	Carboxylic acid derivative
[CX3;!R;$([C][#6]),$([CH]);$([C](=[OX1])[$([SX2H]),$([SX1-])]),$([C](=[SX1])[$([OX2H]),$([OX1-])])]	Carbothioic acid
[CX3;$([R0][#6]),$([H1R0])](=[OX1])[SX2][#6;!$(C=[O,N,S])]	Carbothioic S ester
[#6][#6X3R](=[OX1])[#16X2][#6;!$(C=[O,N,S])]	Carbothioic S lactone
[CX3;$([H0][#6]),$([H1])](=[SX1])[OX2][#6;!$(C=[O,N,S])]	Carbothioic O ester
[#6][#6X3R](=[SX1])[#8X2][#6;!$(C=[O,N,S])]	Carbothioic O lactone
[CX3;$([H0][#6]),$([H1])](=[SX1])[FX1,ClX1,BrX1,IX1]	Carbothioic halide
[CX3;!R;$([C][#6]),$([CH]);$([C](=[SX1])[SX2H])]	Carbodithioic acid
[CX3;!R;$([C][#6]),$([CH]);$([C](=[SX1])[SX2][#6;!$(C=[O,N,S])])]	Carbodithioic ester
[#6][#6X3R](=[SX1])[#16X2][#6;!$(C=[O,N,S])]	Carbodithiolactone
[CX3;$([R0][#6]),$([H1R0])](=[OX1])[#7X3;$([H2]),$([H1][#6;!$(C=[O,N,S])]),$([#7]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])]	Amide
[CX3;$([R0][#6]),$([H1R0])](=[OX1])[NX3H2]	Primary amide
[CX3;$([R0][#6]),$([H1R0])](=[OX1])[#7X3H1][#6;!$(C=[O,N,S])]	Secondary amide
[CX3;$([R0][#6]),$([H1R0])](=[OX1])[#7X3H0]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])]	Tertiary amide
[#6R][#6X3R](=[OX1])[#7X3;$([H1][#6;!$(C=[O,N,S])]),$([H0]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])]	Lactam
[#6X3;$([H0][#6]),$([H1])](=[OX1])[#7X3H0]([#6])[#6X3;$([H0][#6]),$([H1])](=[OX1])	Alkyl imide
[#6X3;$([H0][#6]),$([H1])](=[OX1])[#7X3H0]([!#6])[#6X3;$([H0][#6]),$([H1])](=[OX1])	N hetero imide
[#6X3;$([H0][#6]),$([H1])](=[OX1])[#7X3H1][#6X3;$([H0][#6]),$([H1])](=[OX1])	Imide acidic
[$([CX3;!R][#6]),$([CX3H;!R])](=[SX1])[#7X3;$([H2]),$([H1][#6;!$(C=[O,N,S])]),$([#7]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])]	Thioamide
[#6R][#6X3R](=[SX1])[#7X3;$([H1][#6;!$(C=[O,N,S])]),$([H0]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])]	Thiolactam
[#6X3;$([H0][#6]),$([H1])](=[OX1])[#8X2][#7X2]=,:[#6X3;$([H0]([#6])[#6]),$([H1][#6]),$([H2])]	Oximester
[NX3;!$(NC=[O,S])][CX3;$([CH]),$([C][#6])]=[NX2;!$(NC=[O,S])]	Amidine
[CX3;$([H0][#6]),$([H1])](=[OX1])[#7X3;$([H1]),$([H0][#6;!$(C=[O,N,S])])][$([OX2H]),$([OX1-])]	Hydroxamic acid
[CX3;$([H0][#6]),$([H1])](=[OX1])[#7X3;$([H1]),$([H0][#6;!$(C=[O,N,S])])][OX2][#6;!$(C=[O,N,S])]	Hydroxamic acid ester
[CX3R0;$([H0][#6]),$([H1])](=[NX2;$([H1]),$([H0][#6;!$(C=[O,N,S])])])[$([OX2H]),$([OX1-])]	Imidoacid
[#6R][#6X3R](=,:[#7X2;$([H1]),$([H0][#6;!$(C=[O,N,S])])])[$([OX2H]),$([OX1-])]	Imidoacid cyclic
[CX3R0;$([H0][#6]),$([H1])](=[NX2;$([H1]),$([H0][#6;!$(C=[O,N,S])])])[OX2][#6;!$(C=[O,N,S])]	Imidoester
[#6R][#6X3R](=,:[#7X2;$([H1]),$([H0][#6;!$(C=[O,N,S])])])[OX2][#6;!$(C=[O,N,S])]	Imidolactone
[CX3R0;$([H0][#6]),$([H1])](=[NX2;$([H1]),$([H0][#6;!$(C=[O,N,S])])])[$([SX2H]),$([SX1-])]	Imidothioacid
[#6R][#6X3R](=,:[#7X2;$([H1]),$([H0][#6;!$(C=[O,N,S])])])[$([SX2H]),$([SX1-])]	Imidothioacid cyclic
[CX3R0;$([H0][#6]),$([H1])](=[NX2;$([H1]),$([H0][#6;!$(C=[O,N,S])])])[SX2][#6;!$(C=[O,N,S])]	Imidothioester
[#6R][#6X3R](=,:[#7X2;$([H1]),$([H0][#6;!$(C=[O,N,S])])])[SX2][#6;!$(C=[O,N,S])]	Imidothiolactone
[#7X3v3;!$(N([#6X3]=[#7X2])C=[O,S])][CX3R0;$([H1]),$([H0][#6])]=[NX2v3;!$(N(=[#6X3][#7X3])C=[O,S])]	Amidine
[#6][#6X3R;$([H0](=[NX2;!$(N(=[#6X3][#7X3])C=[O,S])])[#7X3;!$(N([#6X3]=[#7X2])C=[O,S])]),$([H0](-[NX3;!$(N([#6X3]=[#7X2])C=[O,S])])=,:[#7X2;!$(N(=[#6X3][#7X3])C=[O,S])])]	Imidolactam
[CX3R0;$([H0][#6]),$([H1])](=[NX2;$([H1]),$([H0][#6;!$(C=[O,N,S])])])[FX1,ClX1,BrX1,IX1]	Imidoylhalide
[#6R][#6X3R](=,:[#7X2;$([H1]),$([H0][#6;!$(C=[O,N,S])])])[FX1,ClX1,BrX1,IX1]	Imidoylhalide cyclic
[$([$([#6X3][#6]),$([#6X3H])](=[#7X2v3])[#7X3v3][#7X3v3]),$([$([#6X3][#6]),$([#6X3H])]([#7X3v3])=[#7X2v3][#7X3v3])]	Amidrazone
[NX3,NX4+;!$([N]~[!#6]);!$([N]*~[#7,#8,#15,#16])][C][CX3](=[OX1])[OX2H,OX1-]	Alpha aminoacid
[OX2H][C][CX3](=[OX1])[OX2H,OX1-]	Alpha hydroxyacid
[NX3;$([N][CX3](=[OX1])[C][NX3,NX4+])][C][CX3](=[OX1])[NX3;$([N][C][CX3](=[OX1])[NX3,OX2,OX1-])]	Peptide middle
[NX3;$([N][CX3](=[OX1])[C][NX3,NX4+])][C][CX3](=[OX1])[OX2H,OX1-]	Peptide C term
[NX3,NX4+;!$([N]~[!#6]);!$([N]*~[#7,#8,#15,#16])][C][CX3](=[OX1])[NX3;$([N][C][CX3](=[OX1])[NX3,OX2,OX1-])]	Peptide N term
[#6][OX2][CX4;$(C[#6]),$([CH])]([OX2][#6])[OX2][#6]	Carboxylic orthoester
[CX3]=[CX2]=[OX1]	Ketene
[#7X2,#8X3,#16X2;$(*[#6,#14])][#6X3]([#7X2,#8X3,#16X2;$(*[#6,#14])])=[#6X3]	Ketenacetal
[NX1]#[CX2]	Nitrile
[CX1-]#[NX2+]	Isonitrile
[#6X3](=[OX1])[#6X3]=,:[#6X3][#7,#8,#16,F,Cl,Br,I]	Vinylogous carbonyl or carboxyl derivative
[#6X3](=[OX1])[#6X3]=,:[#6X3][$([OX2H]),$([OX1-])]	Vinylogous acid
[#6X3](=[OX1])[#6X3]=,:[#6X3][#6;!$(C=[O,N,S])]	Vinylogous ester
[#6X3](=[OX1])[#6X3]=,:[#6X3][#7X3;$([H2]),$([H1][#6;!$(C=[O,N,S])]),$([#7]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])]	Vinylogous amide
[#6X3](=[OX1])[#6X3]=,:[#6X3][FX1,ClX1,BrX1,IX1]	Vinylogous halide
[#6;!$(C=[O,N,S])][#8X2][#6X3](=[OX1])[#8X2][#6;!$(C=[O,N,S])]	Carbonic acid dieester
[#6;!$(C=[O,N,S])][OX2;!R][CX3](=[OX1])[OX2][FX1,ClX1,BrX1,IX1]	Carbonic acid esterhalide
[#6;!$(C=[O,N,S])][OX2;!R][CX3](=[OX1])[$([OX2H]),$([OX1-])]	Carbonic acid monoester
[!#6][#6X3](=[!#6])[!#6]	Carbonic acid derivatives
[#6;!$(C=[O,N,S])][#8X2][#6X3](=[SX1])[#8X2][#6;!$(C=[O,N,S])]	Thiocarbonic acid dieester
[#6;!$(C=[O,N,S])][OX2;!R][CX3](=[SX1])[OX2][FX1,ClX1,BrX1,IX1]	Thiocarbonic acid esterhalide
[#6;!$(C=[O,N,S])][OX2;!R][CX3](=[SX1])[$([OX2H]),$([OX1-])]	Thiocarbonic acid monoester
[#7X3;!$([#7][!#6])][#6X3](=[OX1])[#7X3;!$([#7][!#6])]	Urea
[#7X3;!$([#7][!#6])][#6X3](=[SX1])[#7X3;!$([#7][!#6])]	Thiourea
[#7X2;!$([#7][!#6])]=,:[#6X3]([#8X2;!$([#8][!#6]),OX1-])[#7X3;!$([#7][!#6])]	Isourea
[#7X2;!$([#7][!#6])]=,:[#6X3]([#16X2;!$([#16][!#6]),SX1-])[#7X3;!$([#7][!#6])]	Isothiourea
[N;v3X3,v4X4+][CX3](=[N;v3X2,v4X3+])[N;v3X3,v4X4+]	Guanidine
[NX3]C(=[OX1])[O;X2H,X1-]	Carbaminic acid
[#7X3][#6](=[OX1])[#8X2][#6]	Urethan
[#7X3][#6](=[OX1])[#7X3][#6](=[OX1])[#7X3]	Biuret
[#7X3][#7X3][#6X3]([#7X3;!$([#7][#7])])=[OX1]	Semicarbazide
[#7X3][#7X3][#6X3]([#7X3][#7X3])=[OX1]	Carbazide
[#7X2](=[#6])[#7X3][#6X3]([#7X3;!$([#7][#7])])=[OX1]	Semicarbazone
[#7X2](=[#6])[#7X3][#6X3]([#7X3][#7X3])=[OX1]	Carbazone
[#7X3][#7X3][#6X3]([#7X3;!$([#7][#7])])=[SX1]	Thiosemicarbazide
[#7X3][#7X3][#6X3]([#7X3][#7X3])=[SX1]	Thiocarbazide
[#7X2](=[#6])[#7X3][#6X3]([#7X3;!$([#7][#7])])=[SX1]	Thiosemicarbazone
[#7X2](=[#6])[#7X3][#6X3]([#7X3][#7X3])=[SX1]	Thiocarbazone
[NX2]=[CX2]=[OX1]	Isocyanate
[OX2][CX2]#[NX1]	Cyanate
[NX2]=[CX2]=[SX1]	Isothiocyanate
[SX2][CX2]#[NX1]	Thiocyanate
[NX2]=[CX2]=[NX2]	Carbodiimide
[CX4H0]([O,S,#7])([O,S,#7])([O,S,#7])[O,S,#7,F,Cl,Br,I]	Orthocarbonic derivatives
[OX2H][c]	Phenol
[OX2H][c][c][OX2H]	1,2-Diphenol
[Cl][c]	Arylchloride
[F][c]	Arylfluoride
[Br][c]	Arylbromide
[I][c]	Aryliodide
[SX2H][c]	Arylthiol
[c]=[NX2;$([H1]),$([H0][#6;!$([C]=[N,S,O])])]	Iminoarene
[c]=[OX1]	Oxoarene
[c]=[SX1]	Thioarene
[nX3H1+0]	Hetero N basic H
[nX3H0+0]	Hetero N basic no H
[nX2,nX3+]	Hetero N nonbasic
[o]	Hetero O
[sX2]	Hetero S
[a;!c]	Heteroaromatic
[NX2](=[OX1])[O;$([X2]),$([X1-])]	Nitrite
[SX2][NX2]=[OX1]	Thionitrite
[$([NX3](=[OX1])(=[OX1])[O;$([X2]),$([X1-])]),$([NX3+]([OX1-])(=[OX1])[O;$([X2]),$([X1-])])]	Nitrate
[$([NX3](=O)=O),$([NX3+](=O)[O-])][!#8]	Nitro
[NX2](=[OX1])[!#7;!#8]	Nitroso
[NX1]~[NX2]~[NX2,NX1]	Azide
[CX3](=[OX1])[NX2]~[NX2]~[NX1]	Acylazide
[$([#6]=[NX2+]=[NX1-]),$([#6-]-[NX2+]#[NX1])]	Diazo
[#6][NX2+]#[NX1]	Diazonium
[#7;!$(N*=O)][NX2]=[OX1]	Nitrosamine
[NX2](=[OX1])N-*=O	Nitrosamide
[$([#7+][OX1-]),$([#7v5]=[OX1]);!$([#7](~[O])~[O]);!$([#7]=[#7])]	N-Oxide
[NX3;$([H2]),$([H1][#6]),$([H0]([#6])[#6]);!$(NC=[O,N,S])][NX3;$([H2]),$([H1][#6]),$([H0]([#6])[#6]);!$(NC=[O,N,S])]	Hydrazine
[NX3;$([H2]),$([H1][#6]),$([H0]([#6])[#6]);!$(NC=[O,N,S])][NX2]=[#6]	Hydrazone
[NX3;$([H2]),$([H1][#6]),$([H0]([#6])[#6]);!$(NC=[O,N,S])][OX2;$([H1]),$(O[#6;!$(C=[N,O,S])])]	Hydroxylamine
[$([SX4](=[OX1])(=[OX1])([#6])[#6]),$([SX4+2]([OX1-])([OX1-])([#6])[#6])]	Sulfon
[$([SX3](=[OX1])([#6])[#6]),$([SX3+]([OX1-])([#6])[#6])]	Sulfoxide
[S+;!$([S]~[!#6]);!$([S]*~[#7,#8,#15,#16])]	Sulfonium
[SX4](=[OX1])(=[OX1])([$([OX2H]),$([OX1-])])[$([OX2H]),$([OX1-])]	Sulfuric acid
[SX4](=[OX1])(=[OX1])([$([OX2H]),$([OX1-])])[OX2][#6;!$(C=[O,N,S])]	Sulfuric monoester
[SX4](=[OX1])(=[OX1])([OX2][#6;!$(C=[O,N,S])])[OX2][#6;!$(C=[O,N,S])]	Sulfuric diester
[SX4](=[OX1])(=[OX1])([#7X3;$([H2]),$([H1][#6;!$(C=[O,N,S])]),$([#7]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])])[$([OX2H]),$([OX1-])]	Sulfuric monoamide
[SX4](=[OX1])(=[OX1])([#7X3;$([H2]),$([H1][#6;!$(C=[O,N,S])]),$([#7]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])])[#7X3;$([H2]),$([H1][#6;!$(C=[O,N,S])]),$([#7]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])]	Sulfuric diamide
[SX4](=[OX1])(=[OX1])([#7X3][#6;!$(C=[O,N,S])])[OX2][#6;!$(C=[O,N,S])]	Sulfuric esteramide
[SX4D4](=[!#6])(=[!#6])([!#6])[!#6]	Sulfuric derivative
[SX4;$([H1]),$([H0][#6])](=[OX1])(=[OX1])[$([OX2H]),$([OX1-])]	Sulfonic acid
[SX4;$([H1]),$([H0][#6])](=[OX1])(=[OX1])[#7X3;$([H2]),$([H1][#6;!$(C=[O,N,S])]),$([#7]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])]	Sulfonamide
[SX4;$([H1]),$([H0][#6])](=[OX1])(=[OX1])[OX2][#6;!$(C=[O,N,S])]	Sulfonic ester
[SX4;$([H1]),$([H0][#6])](=[OX1])(=[OX1])[FX1,ClX1,BrX1,IX1]	Sulfonic halide
[SX4;$([H1]),$([H0][#6])](=[!#6])(=[!#6])[!#6]	Sulfonic derivative
[SX3;$([H1]),$([H0][#6])](=[OX1])[$([OX2H]),$([OX1-])]	Sulfinic acid
[SX3;$([H1]),$([H0][#6])](=[OX1])[#7X3;$([H2]),$([H1][#6;!$(C=[O,N,S])]),$([#7]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])]	Sulfinic amide
[SX3;$([H1]),$([H0][#6])](=[OX1])[OX2][#6;!$(C=[O,N,S])]	Sulfinic ester
[SX3;$([H1]),$([H0][#6])](=[OX1])[FX1,ClX1,BrX1,IX1]	Sulfinic halide
[SX3;$([H1]),$([H0][#6])](=[!#6])[!#6]	Sulfinic derivative
[SX2;$([H1]),$([H0][#6])][$([OX2H]),$([OX1-])]	Sulfenic acid
[SX2;$([H1]),$([H0][#6])][#7X3;$([H2]),$([H1][#6;!$(C=[O,N,S])]),$([#7]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])]	Sulfenic amide
[SX2;$([H1]),$([H0][#6])][OX2][#6;!$(C=[O,N,S])]	Sulfenic ester
[SX2;$([H1]),$([H0][#6])][FX1,ClX1,BrX1,IX1]	Sulfenic halide
[SX2;$([H1]),$([H0][#6])][!#6]	Sulfenic derivative
[PX3;$([H3]),$([H2][#6]),$([H1]([#6])[#6]),$([H0]([#6])([#6])[#6])]	Phosphine
[PX4;$([H3]=[OX1]),$([H2](=[OX1])[#6]),$([H1](=[OX1])([#6])[#6]),$([H0](=[OX1])([#6])([#6])[#6])]	Phosphine oxide
[P+;!$([P]~[!#6]);!$([P]*~[#7,#8,#15,#16])]	Phosphonium
[PX4;$([H3]=[CX3]),$([H2](=[CX3])[#6]),$([H1](=[CX3])([#6])[#6]),$([H0](=[CX3])([#6])([#6])[#6])]	Phosphorylen
[PX4;$([H1]),$([H0][#6])](=[OX1])([$([OX2H]),$([OX1-])])[$([OX2H]),$([OX1-])]	Phosphonic acid
[PX4;$([H1]),$([H0][#6])](=[OX1])([$([OX2H]),$([OX1-])])[OX2][#6;!$(C=[O,N,S])]	Phosphonic monoester
[PX4;$([H1]),$([H0][#6])](=[OX1])([OX2][#6;!$(C=[O,N,S])])[OX2][#6;!$(C=[O,N,S])]	Phosphonic diester
[PX4;$([H1]),$([H0][#6])](=[OX1])([$([OX2H]),$([OX1-])])[#7X3;$([H2]),$([H1][#6;!$(C=[O,N,S])]),$([#7]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])]	Phosphonic monoamide
[PX4;$([H1]),$([H0][#6])](=[OX1])([#7X3;$([H2]),$([H1][#6;!$(C=[O,N,S])]),$([#7]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])])[#7X3;$([H2]),$([H1][#6;!$(C=[O,N,S])]),$([#7]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])]	Phosphonic diamide
[PX4;$([H1]),$([H0][#6])](=[OX1])([OX2][#6;!$(C=[O,N,S])])[#7X3;$([H2]),$([H1][#6;!$(C=[O,N,S])]),$([#7]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])]	Phosphonic esteramide
[PX4;$([H1]),$([H0][#6])](=[!#6])([!#6])[!#6]	Phosphonic acid derivative
[PX4D4](=[OX1])([$([OX2H]),$([OX1-])])([$([OX2H]),$([OX1-])])[$([OX2H]),$([OX1-])]	Phosphoric acid
[PX4D4](=[OX1])([$([OX2H]),$([OX1-])])([$([OX2H]),$([OX1-])])[OX2][#6;!$(C=[O,N,S])]	Phosphoric monoester
[PX4D4](=[OX1])([$([OX2H]),$([OX1-])])([OX2][#6;!$(C=[O,N,S])])[OX2][#6;!$(C=[O,N,S])]	Phosphoric diester
[PX4D4](=[OX1])([OX2][#6;!$(C=[O,N,S])])([OX2][#6;!$(C=[O,N,S])])[OX2][#6;!$(C=[O,N,S])]	Phosphoric triester
[PX4D4](=[OX1])([$([OX2H]),$([OX1-])])([$([OX2H]),$([OX1-])])[#7X3;$([H2]),$([H1][#6;!$(C=[O,N,S])]),$([#7]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])]	Phosphoric monoamide
[PX4D4](=[OX1])([$([OX2H]),$([OX1-])])([#7X3;$([H2]),$([H1][#6;!$(C=[O,N,S])]),$([#7]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])])[#7X3;$([H2]),$([H1][#6;!$(C=[O,N,S])]),$([#7]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])]	Phosphoric diamide
[PX4D4](=[OX1])([#7X3;$([H2]),$([H1][#6;!$(C=[O,N,S])]),$([#7]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])])([#7X3;$([H2]),$([H1][#6;!$(C=[O,N,S])]),$([#7]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])])[#7X3;$([H2]),$([H1][#6;!$(C=[O,N,S])]),$([#7]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])]	Phosphoric triamide
[PX4D4](=[OX1])([$([OX2H]),$([OX1-])])([OX2][#6;!$(C=[O,N,S])])[#7X3;$([H2]),$([H1][#6;!$(C=[O,N,S])]),$([#7]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])]	Phosphoric monoestermonoamide
[PX4D4](=[OX1])([OX2][#6;!$(C=[O,N,S])])([OX2][#6;!$(C=[O,N,S])])[#7X3;$([H2]),$([H1][#6;!$(C=[O,N,S])]),$([#7]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])]	Phosphoric diestermonoamide
[PX4D4](=[OX1])([OX2][#6;!$(C=[O,N,S])])([#7X3;$([H2]),$([H1][#6;!$(C=[O,N,S])]),$([#7]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])])[#7X3;$([H2]),$([H1][#6;!$(C=[O,N,S])]),$([#7]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])]	Phosphoric monoesterdiamide
[PX4D4](=[!#6])([!#6])([!#6])[!#6]	Phosphoric acid derivative
[PX4;$([H2]),$([H1][#6]),$([H0]([#6])[#6])](=[OX1])[$([OX2H]),$([OX1-])]	Phosphinic acid
[PX4;$([H2]),$([H1][#6]),$([H0]([#6])[#6])](=[OX1])[OX2][#6;!$(C=[O,N,S])]	Phosphinic ester
[PX4;$([H2]),$([H1][#6]),$([H0]([#6])[#6])](=[OX1])[#7X3;$([H2]),$([H1][#6;!$(C=[O,N,S])]),$([#7]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])]	Phosphinic amide
[PX4;$([H2]),$([H1][#6]),$([H0]([#6])[#6])](=[!#6])[!#6]	Phosphinic acid derivative
[PX3;$([H1]),$([H0][#6])]([$([OX2H]),$([OX1-])])[$([OX2H]),$([OX1-])]	Phosphonous acid
[PX3;$([H1]),$([H0][#6])]([$([OX2H]),$([OX1-])])[OX2][#6;!$(C=[O,N,S])]	Phosphonous monoester
[PX3;$([H1]),$([H0][#6])]([OX2][#6;!$(C=[O,N,S])])[OX2][#6;!$(C=[O,N,S])]	Phosphonous diester
[PX3;$([H1]),$([H0][#6])]([$([OX2H]),$([OX1-])])[#7X3;$([H2]),$([H1][#6;!$(C=[O,N,S])]),$([#7]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])]	Phosphonous monoamide
[PX3;$([H1]),$([H0][#6])]([#7X3;$([H2]),$([H1][#6;!$(C=[O,N,S])]),$([#7]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])])[#7X3;$([H2]),$([H1][#6;!$(C=[O,N,S])]),$([#7]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])]	Phosphonous diamide
[PX3;$([H1]),$([H0][#6])]([OX2][#6;!$(C=[O,N,S])])[#7X3;$([H2]),$([H1][#6;!$(C=[O,N,S])]),$([#7]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])]	Phosphonous esteramide
[PX3;$([D2]),$([D3][#6])]([!#6])[!#6]	Phosphonous derivatives
[PX3;$([H2]),$([H1][#6]),$([H0]([#6])[#6])][$([OX2H]),$([OX1-])]	Phosphinous acid
[PX3;$([H2]),$([H1][#6]),$([H0]([#6])[#6])][OX2][#6;!$(C=[O,N,S])]	Phosphinous ester
[PX3;$([H2]),$([H1][#6]),$([H0]([#6])[#6])][#7X3;$([H2]),$([H1][#6;!$(C=[O,N,S])]),$([#7]([#6;!$(C=[O,N,S])])[#6;!$(C=[O,N,S])])]	Phosphinous amide
[PX3;$([H2]),$([H1][#6]),$([H0]([#6])[#6])][!#6]	Phosphinous derivatives
[SiX4]([#6])([#6])([#6])[#6]	Quart silane
[SiX4;$([H1]([#6])([#6])[#6]),$([H2]([#6])[#6]),$([H3][#6]),$([H4])]	Non-quart silane
[SiX4]([FX1,ClX1,BrX1,IX1])([#6])([#6])[#6]	Silylmonohalide
[SiX4]([!#6])([#6])([#6])[#6]	Het trialkylsilane
[SiX4]([!#6])([!#6])([#6])[#6]	Dihet dialkylsilane
[SiX4]([!#6])([!#6])([!#6])[#6]	Trihet alkylsilane
[SiX4]([!#6])([!#6])([!#6])[!#6]	Silicic acid derivative
[BX3]([#6])([#6])[#6]	Trialkylborane
[BX3]([!#6])([!#6])[!#6]	Boric acid derivatives
[BX3]([!#6])([!#6])[!#6]	Boronic acid derivative
[BH1,BH2,BH3,BH4]	Borohydride
[BX4]	Quaternary boron
a	Aromatic
[!#6;!R0]	Heterocyclic
[OX2r3]1[#6r3][#6r3]1	Epoxide
[NX3H1r3]1[#6r3][#6r3]1	NH aziridine
[D4R;$(*(@*)(@*)(@*)@*)]	Spiro
[R;$(*(@*)(@*)@*);!$([R2;$(*(@*)(@*)(@*)@*)])]@[R;$(*(@*)(@*)@*);!$([R2;$(*(@*)(@*)(@*)@*)])]	Annelated rings
[R;$(*(@*)(@*)@*);!$([D4R;$(*(@*)(@*)(@*)@*)]);!$([R;$(*(@*)(@*)@*);!$([R2;$(*(@*)(@*)(@*)@*)])]@[R;$(*(@*)(@*)@*);!$([R2;$(*(@*)(@*)(@*)@*)])])]	Bridged rings
[OX2;$([r5]1@C@C@C(O)@C1),$([r6]1@C@C@C(O)@C(O)@C1)]	Sugar pattern 1
[OX2;$([r5]1@C(!@[OX2,NX3,SX2,FX1,ClX1,BrX1,IX1])@C@C@C1),$([r6]1@C(!@[OX2,NX3,SX2,FX1,ClX1,BrX1,IX1])@C@C@C@C1)]	Sugar pattern 2
[OX2;$([r5]1@C(!@[OX2,NX3,SX2,FX1,ClX1,BrX1,IX1])@C@C(O)@C1),$([r6]1@C(!@[OX2,NX3,SX2,FX1,ClX1,BrX1,IX1])@C@C(O)@C(O)@C1)]	Sugar pattern combi
[OX2;$([r5]1@C(!@[OX2H1])@C@C@C1),$([r6]1@C(!@[OX2H1])@C@C@C@C1)]	Sugar pattern 2 reducing
[OX2;$([r5]1@[C@@](!@[OX2,NX3,SX2,FX1,ClX1,BrX1,IX1])@C@C@C1),$([r6]1@[C@@](!@[OX2,NX3,SX2,FX1,ClX1,BrX1,IX1])@C@C@C@C1)]	Sugar pattern 2 alpha
[OX2;$([r5]1@[C@](!@[OX2,NX3,SX2,FX1,ClX1,BrX1,IX1])@C@C@C1),$([r6]1@[C@](!@[OX2,NX3,SX2,FX1,ClX1,BrX1,IX1])@C@C@C@C1)]	Sugar pattern 2 beta
*=*[*]=,#,:[*]	Conjugated double bond
*#*[*]=,#,:[*]	Conjugated tripple bond
*/[D2]=[D2]\*    Cis double bond
*/[D2]=[D2]/*	Trans double bond
[$(*=O),$([#16,#14,#5]),$([#7]([#6]=[OX1]))][#8X2][$(*=O),$([#16,#14,#5]),$([#7]([#6]=[OX1]))]	Mixed anhydrides
[FX1,ClX1,BrX1,IX1][!#6]	Halogen on hetero
[F,Cl,Br,I;!$([X1]);!$([X0-])]	Halogen multi subst
[FX1][CX4;!$([H0][Cl,Br,I]);!$([F][C]([F])([F])[F])]([FX1])([FX1])	Trifluoromethyl
[#6]~[#7,#8,#16]	C ONS bond
[!+0]	Charged
[-1,-2,-3,-4,-5,-6,-7]	Anion
[+1,+2,+3,+4,+5,+6,+7]	Kation
([-1,-2,-3,-4,-5,-6,-7]).([+1,+2,+3,+4,+5,+6,+7])	Salt
[$([#7X2,OX1,SX1]=*[!H0;!$([a;!n])]),$([#7X3,OX2,SX2;!H0]*=*),$([#7X3,OX2,SX2;!H0]*:n)]	1,3-Tautomerizable
[$([#7X2,OX1,SX1]=,:**=,:*[!H0;!$([a;!n])]),$([#7X3,OX2,SX2;!H0]*=**=*),$([#7X3,OX2,SX2;!H0]*=,:**:n)]	1,5-Tautomerizable
[!$(*#*);!D1]-!@[!$(*#*);!D1]	Rotatable bond
[CX3]=[CX3][$([CX3]=[O,N,S]),$(C#[N]),$([S,P]=[OX1]),$([NX3]=O),$([NX3+](=O)[O-])]	Michael acceptor
[CX3](=[OX1])[NX2]=[NX2][CX3](=[OX1])	Dicarbodiazene
[$([CX4;!$([H0]);!$(C[!#6;!$([P,S]=O);!$(N(~O)~O)])][$([CX3]=[O,N,S]),$(C#[N]),$([S,P]=[OX1]),$([NX3]=O),$([NX3+](=O)[O-]);!$(*[S,O,N;H1,H2]);!$([*+0][S,O;X1-])]),$([CX4;!$([H0])]1[CX3]=[CX3][CX3]=[CX3]1)]	CH-acidic
[CX4;!$([H0]);!$(C[!#6;!$([P,S]=O);!$(N(~O)~O)])]([$([CX3]=[O,N,S]),$(C#[N]),$([S,P]=[OX1]),$([NX3]=O),$([NX3+](=O)[O-]);!$(*[S,O,N;H1,H2]);!$([*+0][S,O;X1-])])[$([CX3]=[O,N,S]),$(C#[N]),$([S,P]=[OX1]),$([NX3]=O),$([NX3+](=O)[O-]);!$(*[S,O,N;H1,H2]);!$([*+0][S,O;X1-])]	CH-acidic strong
[$([*@](~*)(~*)(*)*),$([*@H](*)(*)*),$([*@](~*)(*)*),$([*@H](~*)~*)]	Chiral center specified
CO[Na]
