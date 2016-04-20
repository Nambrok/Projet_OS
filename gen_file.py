#coding: utf8
import random as rand
nom = raw_input("Entrer le nom du fichier : ")
#~ nbVal = input("Entrer le nombre de valeur que doit contenir le fichier : ")

fw = open(nom, "w")
nbVal = rand.randint(0, 100000)
fw.write(str(nbVal)+"\n")
i = 0
while(i<nbVal):
	fw.write(str(rand.uniform(0.0, 1000000.0))+"\n")
	i+=1
fw.close()
print("Fichier généré.")
