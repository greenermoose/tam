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

    def add(self, topic, path, title, keywords, kind, track='security', parent='index.html', level=''):
        self.rows.append([topic,path,title,'Read about '+title,keywords,level,track,'','1','2026-11-14 12:00'])
        self.kinds.append(path+'\t'+kind)
        self.parents.append(path+'\t'+parent)

    def save(self):
        (self.root/'index.tsv').write_text('topic\tpath\ttitle\tsummary\tkeywords\tlevel\ttrack\tprereqs\trev\tupdated\n'+''.join('\t'.join(r)+'\n' for r in self.rows))
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

    def test_hub_browser_is_not_a_flat_term_catalog(self):
        self.add('security','security/index.html','Security','security','hub')
        self.add('networking','networking/index.html','Networking','networking','hub','networking')
        self.add('systems','systems/index.html','Systems and administration','systems','hub','systems')
        self.add('word','glossary/w/word.html','Word','word','glossary','glossary','glossary/index.html')
        out=self.run_tam('show','tam:hubs','--pipe')
        self.assertIn('Browse hubs',out)
        self.assertNotIn('Subject Hubs in Your Tamlinux User Guide',out)
        self.assertIn('Hubs are the subject homes',out)
        self.assertIn('Using your system',out)
        self.assertIn('Understanding computing',out)
        self.assertNotIn('Choose a subject',out)
        self.assertIn('Security',out)
        self.assertLess(out.index('Systems and administration'),out.index('Networking'))
        self.assertLess(out.index('Networking'),out.index('Security'))
        self.assertNotIn('Word',out)
        self.assertIn('All pages A',out)
        out=self.run_tam('explain','--pipe')
        self.assertIn('Search for a word',out)
        self.assertNotIn('Keyword Index',out)

    def test_topics_and_explain_topics_share_one_catalog(self):
        self.add('word','glossary/w/word.html','Word','word','glossary')
        self.add('connect','security/connect.html','Connect','connect','how-to')
        shown=self.run_tam('show','tam:topics','--pipe')
        slugs=self.run_tam('explain','topics','--terse').splitlines()
        self.assertIn('word',slugs)
        self.assertIn('connect',slugs)
        for slug in slugs:
            self.assertIn(slug,shown)
        self.assertIn('Topics A to Z',shown)
        self.assertNotIn('Browse hubs',shown)

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

    def test_catalog_level_option(self):
        self.add('security','security/index.html','Security','security','hub','','index.html','novice')
        out_flag=self.run_tam('show','--pipe','--level','expert','tam:catalog')
        out_param=self.run_tam('show','--pipe','tam:catalog?level=expert')
        self.assertEqual(out_flag,out_param)
        self.assertIn('All Pages A to Z (expert)',out_flag)
        self.assertIn('?level=expert',out_flag)
        explicit=self.run_tam('show','--pipe','--level','novice','tam:catalog?level=expert')
        self.assertIn('All Pages A to Z (expert)',explicit)

if __name__=='__main__':unittest.main()
