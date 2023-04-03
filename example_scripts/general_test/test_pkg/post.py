class Post:

    def __init__(self):
        self.titles = []

    def add_post(self, title):
        self.titles.append(title)

    def delete_post(self, title):
        self.titles.remove(title)