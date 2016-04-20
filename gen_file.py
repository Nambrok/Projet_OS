#coding: utf8
import random as rand
nom = raw_input("Entrer le nom du fichier : ")
nbVal = input("Entrer le nombre de valeur que doit contenir le fichier : ")

fw = open(nom, "w")

fw.write(str(nbVal)+"\n")
i = 0
while(i<nbVal):
	fw.write(str(rand.randint(0, 1000000))+"\n")
	i+=1
fw.close()
print("Fichier généré.")
