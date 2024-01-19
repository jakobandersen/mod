post.disableInvokeMake()
smiles("C(C1C(C=O)=C(C)C=1(CO))", printStereoWarnings=False)
smiles("CC1=CC(C)=C1(O)", printStereoWarnings=False)
smiles(R"CN(C)C1=NC(=C2C=CC=C12)N(C)C", printStereoWarnings=False)
smiles(R"CC12=C3(C)[Ru+]456789%101(C1=C5C8(=C6C4=C71N)N)C2(C)C%10(=C39C)C", printStereoWarnings=False)
smiles(R"CC12=C3(C)[Ru+]456789%101(C1=C7(N)C4=C6C8(=C51)N)C2(C)C%10(=C39C)C", printStereoWarnings=False)
smiles(R"CC1=C(CC2=C(C)N=S(=O)(O)N=C2C)C(=NS(=O)(=N1)O)C", printStereoWarnings=False)
smiles(R"CCCCCCCCCCCCCCCCCCOc1ccc2C3=NC4=NC(=NC5=NC(=NC6=NC(=NC(=N3)c2c1)c1cc(OCCCCCCCCCCCCCCCCCC)ccc61)c1ccc(OCCCCCCCCCCCCCCCCCC)cc51)c1cc(OCCCCCCCCCCCCCCCCCC)ccc41", printStereoWarnings=False)
smiles(R"c1ccc(cc1)C1=C2C=CC3=C(c4ccccc4)C4=[N+]5C(=C(c6ccccc6)C6=CC=C7C(=C8C=CC1=[N+]8[Zn@@+2]5([NH+]67)[NH+]23)c1cc[n+](cc1)[Pt+2]([n+]1ccc(cc1)C1=C2C=CC3=[N+]2[Zn@@+2]25[NH+]6C1=CC=C6C(=C1C=CC(=[N+]51)C(=C1C=CC(=C3c3ccccc3)[NH+]21)c1ccccc1)c1ccccc1)([n+]1ccc(cc1)C1=C2C=CC3=C(c5ccccc5)C5=[N+]6C(=C(c7ccccc7)C7=CC=C8C(=C9C=CC1=[N+]9[Zn@+2]6([NH+]23)[NH+]78)c1ccccc1)C=C5)[n+]1ccc(cc1)C1=C2C=CC3=C(c5ccccc5)C5=[N+]6C(=C(c7ccccc7)C7=CC=C8C(=C9C=CC1=[N+]9[Zn@+2]6([NH+]23)[NH+]78)c1ccccc1)C=C5)C=C4", printStereoWarnings=False)
smiles(R"c1ccc(cc1)C1=C2C=CC3=C(c4ccccc4)C4=[N+]5C(=C(c6ccccc6)C6=CC=C7C(=C8C=CC1=[N+]8[Zn@@+2]5([NH+]67)[NH+]23)c1cc[n+](cc1)[Pt@@+2]([n+]1ccc(cc1)C1=C2C=CC3=[N+]2[Zn@@+2]25[NH+]6C1=CC=C6C(=C1C=CC(=[N+]51)C(=C1C=CC(=C3c3ccccc3)[NH+]21)c1ccccc1)c1ccccc1)([n+]1ccc(cc1)C1=C2C=CC3=C(c5ccccc5)C5=[N+]6C(=C(c7ccccc7)C7=CC=C8C(=C9C=CC1=[N+]9[Zn@+2]6([NH+]23)[NH+]78)c1ccccc1)C=C5)[n+]1ccc(cc1)C1=C2C=CC3=C(c5ccccc5)C5=[N+]6C(=C(c7ccccc7)C7=CC=C8C(=C9C=CC1=[N+]9[Zn@+2]6([NH+]23)[NH+]78)c1ccccc1)C=C5)C=C4", printStereoWarnings=False)
smiles(R"NC1=C(/N=N/c2ccc(cc2)[N+](=O)[O-])C(=N[S@](=O)(=N1)O)N", printStereoWarnings=False)
smiles(R"NC1=N[S@](=O)(=NC(=C1/N=N/c1ccc(cc1)[N+](=O)[O-])N)O", printStereoWarnings=False)
smiles(R"[O+]#C[Cr]12345(C#[O+])(C#[O+])C6=C1C3(=C5(N)C4(=C26)C)C", printStereoWarnings=False)
smiles(R"C1C23=C4(C5=C6C7=C2[Ru+2]34567)C23=C4C5=C6C7=C13[Ru+2]24567", printStereoWarnings=False)
smiles(R"C1C23=C4C5=C6C7=C3([Ru+2]24567)C23=C41C1=C5C6=C2[Ru+2]34156", printStereoWarnings=False)
smiles(R"c1ccc2C3=NC4=NC(=NC5=NC(=NC6=NC(=NC(=N3)c2c1)c1ccccc61)c1ccccc51)c1ccccc41", printStereoWarnings=False)
graphLike = GraphPrinter()
molLike = GraphPrinter()
graphLike.withIndex = True
graphLike.collapseHydrogens = True
molLike.setMolDefault()
molLike.withIndex = True
#for a in inputGraphs: a.print(graphLike, molLike, printStereoWarnings=False)
