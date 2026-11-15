"""Discovery behavior against tiny independent indexes, never live page wording."""
import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

REPO = Path(__file__).resolve().parents[1]

class LibraryDiscovery(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        (self.root / "index.html").write_text("<html><body><h1>Fixture library</h1></body></html>")
        self.rows = []
        self.kinds = []
        self.parents = []

    def add(self, topic, path, title, keywords, kind, track='security', parent='index.html'):
        self.rows.append([topic,path,title,'Read about '+title,keywords,track,'','1','2026-11-14 12:00'])
        self.kinds.append(path+'\t'+kind)
        self.parents.append(path+'\t'+parent)

    def save(self):
        (self.root/'index.tsv').write_text('topic\tpath\ttitle\tsummary\tkeywords\ttrack\tprereqs\trev\tupdated\n'+''.join('\t'.join(r)+'\n' for r in self.rows))
        (self.root/'index.kinds.tsv').write_text('path\tkind\n'+'\n'.join(self.kinds)+'\n')
        (self.root/'index.parents.tsv').write_text('path\tparent\n'+'\n'.join(self.parents)+'\n')

    def run_tam(self,*args):
        self.save()
        r=subprocess.run([str(REPO/'build/tam'),*args],env={**os.environ,'TAM_KNOWLEDGE_DIR':str(self.root)},capture_output=True,text=True,timeout=10)
        self.assertEqual(r.returncode,0,r.stderr)
        return r.stdout

    def test_all_matches_ranked_and_each_purpose_visible(self):
        for n in range(90):self.add('term-'+str(n),'glossary/x/t'+str(n)+'.html','Term '+str(n),'shared','glossary')
        self.add('late-task','security/task.html','Shared task','shared','how-to')
        self.add('shared','security/explanation.html','Shared explanation','shared','explanation')
        self.add('course','paths/course.html','Shared course','shared','path','paths')
        result=self.run_tam('show','tam:search?query=shared','--pipe')
        for text in ['Definitions','Practical guidance','Shared task','Explanations and reference','Shared explanation','Learning paths','Shared course','More matches']:
            self.assertIn(text,result)
        ranked=json.loads(self.run_tam('search','--json','shared'))
        self.assertIn('security/explanation.html',json.dumps(ranked))

    def test_kind_metadata_classifies_definition_outside_old_directory(self):
        self.add('ssh','terms/ssh.html','SSH meaning','secure shell, ssh','glossary')
        self.add('connect','security/connect.html','Connect to another computer','secure shell, ssh','how-to')
        out=self.run_tam('show','tam:search?query=secure%20shell','--pipe')
        self.assertLess(out.index('Definitions'),out.index('SSH meaning'))
        self.assertIn('Practical guidance',out)
        self.assertIn('Connect to another computer',out)

    def test_missing_query_and_legacy_corpus(self):
        self.add('glossary-bus','learn/glossary/bus.html','Bus meaning','bus','glossary','learn')
        self.save();(self.root/'index.kinds.tsv').unlink()
        env={**os.environ,'TAM_KNOWLEDGE_DIR':str(self.root)}
        out=subprocess.check_output([str(REPO/'build/tam'),'show','tam:search?query=bus','--pipe'],env=env,text=True)
        self.assertIn('Definitions',out)
        out=self.run_tam('show','tam:search?query=unfindablezz','--pipe')
        self.assertIn('No Guide pages matched',out)
        self.assertIn('Search again',out)

    def test_subject_browser_is_not_a_flat_term_catalog(self):
        self.add('security','security/index.html','Security','security','hub')
        self.add('networking','networking/index.html','Networking','networking','hub')
        self.add('systems','systems/index.html','Operating systems','systems','hub')
        self.add('word','glossary/w/word.html','Word','word','glossary','glossary','glossary/index.html')
        out=self.run_tam('show','tam:topics','--pipe')
        self.assertIn('Browse topics',out)
        self.assertIn('Topics Covered in Your Tamlinux User Guide',out)
        self.assertNotIn('Choose a subject',out)
        self.assertIn('Security',out)
        self.assertLess(out.index('Networking'),out.index('Operating systems'))
        self.assertLess(out.index('Operating systems'),out.index('Security'))
        self.assertNotIn('Word',out)
        self.assertIn('All pages A',out)
        out=self.run_tam('explain','--pipe')
        self.assertIn('Search for a word',out)
        self.assertNotIn('Keyword Index',out)

    def test_catalog_lists_all_pages_alphabetically(self):
        self.add('security','security/index.html','Security','security','hub')
        self.add('networking','networking/index.html','Networking','networking','hub')
        self.add('word','glossary/w/word.html','Word','word','glossary','glossary','glossary/index.html')
        out=self.run_tam('show','tam:catalog','--pipe')
        self.assertIn('Guide > Catalog',out)
        self.assertIn('All Pages A to Z',out)
        self.assertIn('Page',out)
        self.assertIn('Subject Matter',out)
        self.assertNotIn('Every topic tam can explain',out)
        self.assertLess(out.index('Networking'),out.index('Security'))
        self.assertLess(out.index('Security'),out.index('Word'))

if __name__=='__main__':unittest.main()
