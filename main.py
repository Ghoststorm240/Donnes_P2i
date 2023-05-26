#importations
import mysql.connector as mysql
import json

def ouvrir_connexion_bd():
    """
    Fonction qui ouvre une connexion à la base de données
    """
    print("")
    print("***********************")
    print("** Connexion à la BD **")
    print("***********************")

    connexion_bd = None
    try:
        connexion_bd = mysql.connect(
                host="fimi-bd-srv1.insa-lyon.fr",
                port=3306,
                user="G223_C",
                password="G223_C",
                database="G223_C_BD1"
            )
    except Exception as e:
        if type(e) == NameError and str(e).startswith("name 'mysql'"):
            print("[ERROR] MySQL: Driver 'mysql' not installed ? (Python Exception: " + str(e) + ")")
            print("[ERROR] MySQL:" +
                  " Package MySQL Connector should be installed [Terminal >> pip install mysql-connector-python ]" +
                  " and imported in the script [import mysql.connector as mysql]")
        else:
            print("[ERROR] MySQL: " + str(e))

    if connexion_bd is not None:
        print("=> Connexion établie...")
    else:
        print("=> Connexion échouée...")

    return connexion_bd

def fermer_connexion_bd(connexion_bd):
    print("")
    print("Fermeture de la Connexion à la BD")

    if connexion_bd is not None:
        try:
            connexion_bd.close()
            print("=> Connexion fermée...")
        except Exception as e:
            print("[ERROR] MySQL: "+str(e))
    else:
        print("=> pas de Connexion ouverte")

def creation_json():
    # Données des ouvriers
    mesures = {
    }

    # Convertir en JSON
    json_data = json.dumps(mesures, indent=4)

def ajout_ouvrier(nom:str, prenom:str, poste:str, identifiant:int):
    # Charger le JSON existant
    with open('ouvriers.json', 'r') as file:
        data = json.load(file)

    # Ajouter les données du nouvel ouvrier
    nouvel_ouvrier = {
        identifiant: {
            "identite": {
                "nom": nom,
                "prenom": prenom,
                "poste": poste
            },
            "mesures": {

            }
        }
    }

    data.update(nouvel_ouvrier)

    # Enregistrer le JSON mis à jour
    with open('ouvriers.json', 'w') as file:
        json.dump(data, file, indent=4)

def ajout_mesure(jour, liste_valeur:list, type:str, ouvrier:int):
    """
    On passe la liste des valeurs en paramètre. Il ne peut pas il y avoir ce type de variable ce jour là
    déjà dans le JSON car on importe les données une fois en tout.
    En revanche, la date peut être déjà présente dans le JSON, il faut donc vérifier si la date est déjà présente
    La liste de valeur est une liste de tuple (heure, valeur)
    """
    # Charger le JSON existant
    with open('ouvriers.json', 'r') as file:
        data = json.load(file)

    if ouvrier not in data:
        print("L'ouvrier n'est pas dans la base de données")
        return

    # On vérifie si la date est déjà présente dans le JSON
    if jour in data[ouvrier]["mesures"]:
        # La date est déjà présente dans le JSON, on ajoute donc les valeurs à la liste des valeurs déjà présentes
        data[ouvrier]["mesures"][jour][type] = [(valeur[0], valeur[1]) for valeur in liste_valeur]
    else:
        # La date n'est pas présente dans le JSON, on ajoute donc la date et les valeurs
        nouvelle_mesure = {
            jour: {
                type: [(valeur[0], valeur[1]) for valeur in liste_valeur]
            }
        }
        data.update(nouvelle_mesure)

    # Enregistrer le JSON mis à jour
    with open('ouvriers.json', 'w') as file:
        json.dump(data, file, indent=4)

# Import des données
def import_donnees(id_presation):
    creation_json()
    connexion_bd = ouvrir_connexion_bd()
    cursor = connexion_bd.cursor()
    cursor.execute("SELECT * "
                   "FROM Ouvrier, Prestation "
                   "WHERE idPrestation = %s "
                   "AND Ouvrier.idEntreprise = Prestation.idEntreprise ", (id_presation))
    for i in range(len(cursor)):
        ajout_ouvrier(cursor[i][1], cursor[i][2], cursor[i][3], cursor[i][0])
        id_ouvrier = cursor[i][0]
        # /!\ est ce que ça passe pour le type de capteur
        cursor.execute("SELECT Mesure.dateMesure, Mesure.valeur, AppartientA.localisation "
                       "FROM Mesure, AppartientA, Panople, MobilisePour, Entreprise, Porte, Ouvrier "
                       "WHERE idOuvrier = %s "
                       "AND idPrestation = %s "
                       "AND AppartientA.idCapteur = Capteur.idCapteur "
                       "AND AppartientA.idPanople = Porte.idPanople "
                       "AND AppartientA.idPanople = MobilisePour.idPanople "
                       "AND MobilisePour.idPrestation = Prestation.idPrestation "
                       "AND Porte.idOuvrier = Ouvrier.idOuvrier ", (id_ouvrier, id_presation))
        mesures = {}
        for j in range(len(cursor)):
            if cursor[j][2] not in mesures:
                mesures[cursor[j][2]] = []
            jour, heure = cursor[j][0].split()
            mesures[cursor[j][2]].append((heure, cursor[j][1]))
        for type_mesure in mesures.keys():
            liste_valeur = mesures[type_mesure]
            ajout_mesure(jour, liste_valeur, type_mesure, id_ouvrier)

    fermer_connexion_bd(connexion_bd)