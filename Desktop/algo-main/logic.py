import os
import re

class Node:
    def __init__(self, key):
        self.key = key
        self.left = self.right = None
        self.height = 1
        # Stocke les mots validés par l'utilisateur pour ce noyau/racine
        self.derived_words = {} 

class SARF_Logic:
    def __init__(self):
        self.root_tree = None
        self.schemes = {} # Table de hachage pour les schèmes

    def is_arabic_triple(self, text):
        """Vérifie si l'entrée est composée de exactement 3 lettres arabes."""
        return bool(re.match(r'^[\u0621-\u064A]{3}$', text))

    def strip_tashkeel(self, text):
        """Supprime les accents au cas où l'utilisateur en tape par habitude."""
        if not text: return ""
        return re.sub(r'[\u064B-\u0652]', '', text)

    # --- Gestion de l'Arbre AVL (Recherche Rapide) ---
    def get_height(self, node):
        return node.height if node else 0

    def insert_root(self, root, key):
        if not root: return Node(key)
        if key < root.key: root.left = self.insert_root(root.left, key)
        elif key > root.key: root.right = self.insert_root(root.right, key)
        else: return root 

        root.height = 1 + max(self.get_height(root.left), self.get_height(root.right))
        return root

    def search_root(self, root, key):
        if not root or root.key == key: return root
        if key < root.key: return self.search_root(root.left, key)
        return self.search_root(root.right, key)

    def delete_root(self, root, key):
        if not root: return root
        if key < root.key: root.left = self.delete_root(root.left, key)
        elif key > root.key: root.right = self.delete_root(root.right, key)
        else:
            if not root.left: return root.right
            if not root.right: return root.left
            temp = self._min_node(root.right)
            root.key = temp.key
            root.right = self.delete_root(root.right, temp.key)
        return root

    def _min_node(self, node):
        curr = node
        while curr.left: curr = curr.left
        return curr

    # --- Moteur de Transformation ---
    def apply_scheme(self, root_word, scheme_name):
        """Remplace ف , ع , ل par les lettres de la racine."""
        if len(root_word) != 3: return None
        root_word = self.strip_tashkeel(root_word)
        scheme_name = self.strip_tashkeel(scheme_name)
        
        res = ""
        for char in scheme_name:
            if char == 'ف': res += root_word[0]
            elif char == 'ع': res += root_word[1]
            elif char == 'ل': res += root_word[2]
            else: res += char
        return res

    # --- ⚖️ Moteur de Vérification (Mizan) ---
    def verify_morphology(self, word, root_key):
        """Vérifie si le mot correspond à la racine selon les schèmes dispo."""
        node = self.search_root(self.root_tree, root_key)
        if not node: return False, None
        
        word_clean = self.strip_tashkeel(word)
        root_clean = self.strip_tashkeel(root_key)

        for s_name in self.schemes:
            generated = self.apply_scheme(root_clean, s_name)
            if word_clean == self.strip_tashkeel(generated):
                node.derived_words[word_clean] = node.derived_words.get(word_clean, 0) + 1
                return True, s_name
                
        return False, None

    # --- 🕵️ Moteur d'Analyse (Identify) ---
    def identify_word(self, word):
        """Retrouve la racine (le verbe) et le schème à partir d'un mot."""
        results = []
        w_clean = self.strip_tashkeel(word)
        
        for s_name in self.schemes:
            s_clean = self.strip_tashkeel(s_name)
            if len(w_clean) == len(s_clean):
                r1, r2, r3, possible = None, None, None, True
                for wc, sc in zip(w_clean, s_clean):
                    if sc == 'ف': r1 = wc
                    elif sc == 'ع': r2 = wc
                    elif sc == 'ل': r3 = wc
                    elif wc != sc: possible = False; break
                
                if possible and r1 and r2 and r3:
                    root_cand = r1 + r2 + r3
                    if self.search_root(self.root_tree, root_cand):
                        results.append({
                            "root": root_cand, # C'est le verbe (ex: درس)
                            "scheme": s_name,  # C'est le poids (ex: فاعل)
                            "word": word       # Le mot original (ex: دارس)
                        })
        return results

    # --- Gestion des données ---
    def load_data(self, r_path, s_path):
        """Charge les racines et les schèmes depuis les fichiers."""
        if os.path.exists(r_path):
            with open(r_path, 'r', encoding='utf-8') as f:
                for line in f:
                    r = self.strip_tashkeel(line.strip())
                    if self.is_arabic_triple(r): 
                        self.root_tree = self.insert_root(self.root_tree, r)
        
        if os.path.exists(s_path):
            with open(s_path, 'r', encoding='utf-8') as f:
                for line in f:
                    parts = line.strip().split(',')
                    if len(parts) >= 1:
                        s_name = self.strip_tashkeel(parts[0])
                        self.schemes[s_name] = {"cat": parts[1] if len(parts)>1 else ""}

    def save_roots_to_file(self, path):
        """Sauvegarde les racines de l'arbre dans le fichier texte."""
        roots_list = self.get_all_roots_data(self.root_tree, [])
        with open(path, 'w', encoding='utf-8') as f:
            for r_obj in roots_list:
                f.write(r_obj['root'] + '\n')

    def get_all_roots_data(self, node, res):
        """Récupère toutes les racines pour l'affichage."""
        if node:
            self.get_all_roots_data(node.left, res)
            res.append({
                "root": node.key, 
                "derivatives": node.derived_words
            })
            self.get_all_roots_data(node.right, res)
        return res